#include "./techlef.hh"
#include "model.hh"
#include "si2-lef/lefrReader.hpp"
#include <utility/debug/debug.hh>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <vector>

namespace xtaro::parse
{

    static auto setVersion    (lefrCallbackType_e, double number, lefiUserData ud) -> int;
    static auto setLayer      (lefrCallbackType_e, lefiLayer*, lefiUserData) -> int;

    auto TechLef::loadFrom(const std::filesystem::path& path) -> TechLef {
        auto lef = TechLef{};

        auto file_name = path.string();
        auto fp = std::unique_ptr<FILE, decltype(&fclose)>{
            fopen(file_name.c_str(), "r"),
            &fclose
        };

        // Init Si2 parse
        lefrInit();

        // Set the call-back functions.
        lefrSetVersionCbk(setVersion);
        lefrSetLayerCbk(setLayer);

        // Read the LEF file
        auto ret = lefrRead(fp.get(), file_name.c_str(), static_cast<void*>(&lef));
        if (ret != 0) 
            throw std::runtime_error{"An error occured in LEF parse."};

        // Release all memory used by Si2 parse
        lefrReleaseNResetMemory();

        return lef;
    }

    static auto setVersion(lefrCallbackType_e, double number, lefiUserData ud) -> int
    {
        auto techlef = static_cast<TechLef*>(ud);
        techlef->version = number;
        return 0;
    }

    static auto setLayer(lefrCallbackType_e, lefiLayer* si2Layer, lefiUserData ud) -> int
    {
        auto techlef = static_cast<TechLef*>(ud);

        auto layer = TechLayer {};

        // Set attributes.
        layer.name = si2Layer->name();
        layer.mask = si2Layer->mask();
        layer.type = si2Layer->type();

        if (si2Layer->hasDirection()) 
        {
            if (std::strcmp(si2Layer->direction(), "HORIZONTAL") == 0 ||
                std::strcmp(si2Layer->direction(), "horizontal") == 0) 
            {
                layer.dir = TechLayerDir::Horizontal;
            } 
            else 
            if (std::strcmp(si2Layer->direction(), "VERTICAL") == 0 ||
                std::strcmp(si2Layer->direction(), "vertical") == 0) 
            {
                layer.dir = TechLayerDir::Vertical;
            }
        }

        if (si2Layer->hasWidth()) 
            layer.width = si2Layer->width();

        if (si2Layer->hasPitch())
            layer.pitch = si2Layer->pitch();
        // MARK !!!!
        if (si2Layer->hasXYPitch()) 
            layer.pitch = si2Layer->pitchX();

        // TODO spacing
        if (si2Layer->hasSpacingTableOrtho())
        {
            for (int i = 0; i < si2Layer->numSpacingTable(); ++i)
            {
                auto table = si2Layer->spacingTable(i);
                if (table->isParallel())
                {
                    /*
                    
                    SPACINGTABLE 
                        PARALLELRUNLENGTH    0.0000     0.3000     0.9000     1.8000     2.7000     4.0000     
                        WIDTH 0.0000       0.0700     0.0700     0.0700     0.0700     0.0700     0.0700     
                        WIDTH 0.0900       0.0700     0.0900     0.0900     0.0900     0.0900     0.0900     
                        WIDTH 0.2700       0.0700     0.0900     0.2700     0.2700     0.2700     0.2700     
                        WIDTH 0.5000       0.0700     0.0900     0.2700     0.5000     0.5000     0.5000     
                        WIDTH 0.9000       0.0700     0.0900     0.2700     0.5000     0.9000     0.9000     
                        WIDTH 1.5000       0.0700     0.0900     0.2700     0.5000     0.9000     1.5000      ;
                    
                    */

                    auto parallel = table->parallel();

                    auto table = ParallelSpacingTable{};

                    for (int index = 0; index < parallel->numLength(); ++index)
                        table.lengths.emplace_back(parallel->length(index));

                    for (int index = 0; index < parallel->numWidth(); ++index)
                        table.widths.emplace_back(parallel->width(index));

                    for (int row = 0; row < parallel->numWidth(); ++row)
                    {
                        auto values = std::vector<double>{};
                        for (int col = 0; col < parallel->numLength(); ++col) 
                            // MARK
                            values.emplace_back(parallel->widthSpacing(row, col));
                        table.spacings.emplace_back(std::move(values));
                    }

                    layer.spacingTable = std::move(table);
                }
                else if (table->isInfluence())
                {
                    // TODO
                }
            }
        }

        if (si2Layer->hasArea()) {
            layer.area = si2Layer->area();
        }

        // ENCLOSURE
        for (auto i = 0; i < si2Layer->numEnclosure(); ++i) 
        {
            auto overhang1 = si2Layer->enclosureOverhang1(0);
            auto overhang2 = si2Layer->enclosureOverhang1(1);
            auto type = si2Layer->enclosureRule(i);
            if (std::strcmp(type, "ABOVE") == 0 || std::strcmp(type, "above") == 0) 
            {
                layer.enclosureAbove.emplace(std::tuple<double, double>{overhang1, overhang2});
            }
            else if (std::strcmp(type, "BELOW") == 0 || std::strcmp(type, "below") == 0) 
            {
                layer.enclosureBelow.emplace(std::tuple<double, double>{overhang1, overhang2});
            }
        } 

        
        techlef->layers.emplace(layer.name, std::move(layer));

        return 0;
    }

}