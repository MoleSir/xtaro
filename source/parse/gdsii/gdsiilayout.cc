#include "gdsiilayout.hh"
#include "gdsiicoord.hh"
#include "gdsiitransform.hh"
#include "gdsiiprimitives.hh"
#include "gdsiistructrue.hh"
#include "gdsiireader.hh"
#include "gdsiiwriter.hh"
#include "utility/debug/debug.hh"
#include <exception>
#include <sstream>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace xtaro::parse
{

    static auto dbCoordTotransformCoord(const GDSIIDBCoord& coord) -> transform::Coord 
    {
        return {
            static_cast<double>(coord.x),
            static_cast<double>(coord.y),
        };
    }

    GDSIILayout::GDSIILayout() :
        _lib{ std::make_unique<GDSIILibrary>() },
        _rootStrcutrue{},
        _structures{},
        _pinsInfo{}
    {
    }

    GDSIILayout::GDSIILayout(std::string name) : GDSIILayout{}
    {
        this->_rootStrcutrue = std::make_shared<GDSIIStructure>(GDSIIStructure::withName(std::move(name)));
        this->_structures.emplace(this->_rootStrcutrue->name, this->_rootStrcutrue);
    }

    auto GDSIILayout::version(std::int16_t v) -> GDSIILayout& 
    {
        this->_lib->version = v;
        return *this;
    }

    auto GDSIILayout::libname(std::string name) -> GDSIILayout&
    {
        this->_lib->name = std::move(name);
        return *this;
    }

    auto GDSIILayout::reflib(std::array<std::string, 2> libs) -> GDSIILayout&
    {
        this->_lib->reflibs.emplace(std::move(libs));
        return *this;
    }

    auto GDSIILayout::fonts(std::array<std::string, 4> fs) -> GDSIILayout&
    {
        this->_lib->fonts.emplace(std::move(fs));
        return *this;
    }

    auto GDSIILayout::usrunitsPerDBunit(double v) -> GDSIILayout&
    {
        this->_lib->usrunitsPerDBunit = v;
        return *this;
    }

    auto GDSIILayout::metersPerDBunit(double v) -> GDSIILayout&
    {
        this->_lib->metersPerDBunit = v;
        return *this;
    }

    void GDSIILayout::addInstance(
        const std::unique_ptr<GDSIILayout>& layout, 
        const GDSIIMicroCoord& offset, 
        std::optional<GDSIIMirror> mirror, 
        std::optional<double> angle)
    {
        this->addInstance(*(layout.get()), offset, mirror, angle);
    }

    void GDSIILayout::addInstance(
        const GDSIILayout& layout, 
        const GDSIIMicroCoord& offset, 
        std::optional<GDSIIMirror> mirror,
        std::optional<double> angle)
    {
        debug::debugf("Add one instance lauout of '{}' to '{}' in ({}u, {}u)", layout._rootStrcutrue->name, this->_rootStrcutrue->name, offset.x, offset.y);
        if (&layout == this) {
            debug::warning("Add a layout to itself, do nothing!");
            return;
        }

        for (const auto& [structName, strcuture] : layout._structures)
        {
            if (this->_structures.find(structName) == this->_structures.end())
                this->_structures.emplace(structName, strcuture);
        }

        auto coord = this->micronCoordToDatabaseCoord(offset);
        auto sref = std::make_unique<GDSIISref>(layout._rootStrcutrue->name, coord);

        if (mirror.has_value()) 
        {
            switch (*mirror)
            {
                case GDSIIMirror::MirrorY : {
                    sref->sTrans.emplace(GDSIITransFlag {.x_mirror=true, .magnify=false, .rotate=false });
                    break;
                }
                case GDSIIMirror::MirrorX: {
                    sref->sTrans.emplace(GDSIITransFlag {.x_mirror=true, .magnify=false, .rotate=true });
                    sref->angle.emplace(180.);
                    break;
                }
                case GDSIIMirror::MirrorXY: {
                    sref->sTrans.emplace(GDSIITransFlag {.x_mirror=false, .magnify=false, .rotate=true });
                    sref->angle.emplace(180.);
                }
            }
        }

        // MARK: NO ROTATE

        // if (angle.has_value() && *angle != 0)
        // {
        //     sref->sTrans.emplace(GDSIITransFlag {.x_mirror=false, .magnify=false, .rotate=true,});
        //     sref->angle.emplace(angle.value());
        // }

        this->_rootStrcutrue->srefs.emplace_back(std::move(sref));
    }
    
    void GDSIILayout::addRectangle(std::int16_t layer, const GDSIIMicroCoord& offset, double width, double height)
    {
        if (this->_rootStrcutrue.get() == nullptr) {
            debug::exception("No exit root structure");
        }

        auto coord = this->micronCoordToDatabaseCoord(offset);
        auto xDB = coord.x;
        auto yDB = coord.y;
        auto widthDB {this->micronToDatabase(width)};
        auto heightDB {this->micronToDatabase(height)};

        auto xy = std::vector<GDSIIDBCoord>{
            {xDB, yDB}, 
            {xDB + widthDB, yDB}, 
            {xDB + widthDB, yDB + heightDB}, 
            {xDB, yDB + heightDB}, 
            {xDB, yDB}
        };

        this->_rootStrcutrue->boundarys.emplace_back(std::make_unique<GDSIIBoundary>(layer, std::move(xy))); 
    }

    void GDSIILayout::addText(std::int16_t layer, const GDSIIMicroCoord& offset, std::string text)
    {
        if (this->_rootStrcutrue.get() == nullptr) {
            debug::exception("No exit root structure");
        }

        auto coord = this->micronCoordToDatabaseCoord(offset);
        this->_rootStrcutrue->texts.emplace_back(std::make_unique<GDSIIText>(layer, coord, std::move(text)));
    }

    void GDSIILayout::addPath(std::int16_t layer, const std::vector<GDSIIMicroCoord>& coords, double width)
    {
        if (this->_rootStrcutrue.get() == nullptr) {
            debug::exception("No exit root structure");
        }

        auto widthUnit = this->micronToDatabase(width);

        auto coordsUnit {std::vector<GDSIIDBCoord>{}};
        for (const auto& coord : coords)
            coordsUnit.emplace_back( this->micronCoordToDatabaseCoord(coord) );

        this->_rootStrcutrue->paths.emplace_back(std::make_unique<GDSIIPath>(layer, std::move(coordsUnit), widthUnit));
    }

    void GDSIILayout::insertStructures(std::string name, std::shared_ptr<GDSIIStructure> structure)
    {
        this->_structures.emplace(std::move(name), std::move(structure));
    }

    auto GDSIILayout::measureSize() const -> std::tuple<double, double>
    {
        auto structTransInfos = this->getStrcutureTransformInfos();

        // Get all rectangle from each srtuctures
        auto structRects = std::vector<transform::Rect>{};
        for (const auto& info : structTransInfos) 
        {
            auto boundary = this->measureStructureSize(std::get<0>(info), std::get<1>(info));
            if (!boundary.has_value()) 
                continue;
            structRects.emplace_back(*boundary);
        }
      
        auto maxBoundary = transform::unionRectangles(structRects);
        if (!maxBoundary.has_value())
            return {0, 0};
        
        auto [leftBottom, rightUp] {maxBoundary.value()};
        auto [minX, minY] = leftBottom;
        auto [maxX, maxY] = rightUp;
        auto cellWdith { maxX - minX };
        auto cellHeight { maxY - minY };

        return {
            this->databaseToMicron(cellWdith),
            this->databaseToMicron(cellHeight),
        };
    }

    auto GDSIILayout::measureLayerSize(std::int16_t layer) const -> std::tuple<double, double>
    {
        auto structTransInfos = this->getStrcutureTransformInfos();

        // Get all rectangle from each srtuctures in layer
        auto structRects = std::vector<transform::Rect>{};
        for (const auto& info : structTransInfos) 
        {
            auto boundary = this->measureStructureSizeInLayer(layer, std::get<0>(info), std::get<1>(info));
            if (!boundary.has_value()) 
                continue;
            structRects.emplace_back(*boundary);
        }

        auto maxBoundary = transform::unionRectangles(structRects);
        if (!maxBoundary.has_value())
            return {0, 0};
        
        auto [leftBottom, rightUp] {maxBoundary.value()};
        auto [minX, minY] = leftBottom;
        auto [maxX, maxY] = rightUp;
        auto cellWdith { maxX - minX };
        auto cellHeight { maxY - minY };

        return {
            this->databaseToMicron(cellWdith),
            this->databaseToMicron(cellHeight),
        };
    }

    auto GDSIILayout::pinInfo(const std::string& pinName) const -> const std::vector<std::tuple<std::int16_t, GDSIIMicroRect>>&
    try {
        return this->_pinsInfo.at(pinName);
    }
    catch (const std::exception& _) {
        debug::exceptionf("No pin '{}' in layout '{}'", pinName, this->_rootStrcutrue->name);
    }

    auto GDSIILayout::getStrcutureTransformInfos() const -> std::vector<transform::Info>
    {
        auto transformInfo = std::vector<transform::Info>{};
        auto transformMats = std::vector<transform::Matrix>{};

        this->doGetStrcutureTransformInfos(
            this->_rootStrcutrue->name, 
            GDSIIDBCoord{0, 0},
            0.0,
            1.0,
            transformMats,
            transformInfo
        );

        return transformInfo;
    }

    auto GDSIILayout::doGetStrcutureTransformInfos(
        std::string structName,
        const GDSIIDBCoord& offset,
        double angle,
        double mag,
        std::vector<transform::Matrix>& transformMatrixs,
        std::vector<transform::Info>& transformInfos) const -> void
    {
        angle = angle / 180.0 * 3.14159;

        auto rotateMatrix { transform::Matrix {
            std::array<double, 3>{cos(angle), -1.0 * sin(angle), 0.0},
            std::array<double, 3>{sin(angle), cos(angle), 0.0},
            std::array<double, 3>{0.0, 0.0, 1.0},
        } };

        auto scaleMatrix { transform::Matrix {
            std::array<double, 3>{mag, 0.0, 0.0},
            std::array<double, 3>{0.0, mag, 0.0},
            std::array<double, 3>{0.0, 0.0, mag},
        } };

        auto coord = dbCoordTotransformCoord(offset);
        auto translateMatrix { transform::Matrix {
            std::array<double, 3>{1.0, 0.0, std::get<0>(coord)},
            std::array<double, 3>{0.0, 1.0, std::get<1>(coord)},
            std::array<double, 3>{0.0, 0.0, 1.0},
        } };

        auto transformMatrix { transform::Matrix {
            std::array<double, 3>{1.0, 0.0, 0.0},
            std::array<double, 3>{0.0, 1.0, 0.0},
            std::array<double, 3>{0.0, 0.0, 1.0},
        } };

        transformMatrix = transform::dot(rotateMatrix, transformMatrix);
        transformMatrix = transform::dot(scaleMatrix, transformMatrix);
        transformMatrix = transform::dot(translateMatrix, transformMatrix);
        
        if (!transformMatrixs.empty())
            transformMatrix = transform::dot(transformMatrixs.back(), transformMatrix);

        transformMatrixs.emplace_back(transformMatrix);
        transformInfos.emplace_back(structName, transformMatrix);

        for (const auto& sref : this->_structures.at(structName)->srefs)
        {
            this->doGetStrcutureTransformInfos(
                sref->sName,
                sref->xy.front(),
                sref->angle.has_value() ? sref->angle.value() : 0.0,
                sref->mag.has_value() ? sref->mag.value() : 1.0,
                transformMatrixs,
                transformInfos
            );
        }

        transformMatrixs.pop_back();
    }

    auto GDSIILayout::measureStructureSize(
        const std::string& sName,
        const transform::Matrix& matrix
    ) const -> std::optional<transform::Rect>
    {
        const auto& boundarys = this->_structures.at(sName)->boundarys;
        auto rects = std::vector<transform::Rect>{};

        std::transform(
            boundarys.begin(), boundarys.end(),
            std::back_inserter(rects),
            [this, &matrix] (const std::unique_ptr<GDSIIBoundary>& boundary) {
                auto rect = transform::Rect {
                    dbCoordTotransformCoord(boundary->xy.at(0)),
                    dbCoordTotransformCoord(boundary->xy.at(2))
                };
                return transform::transformRectangle(rect, matrix);
            }
        );

        return transform::unionRectangles(rects);
    }

    auto GDSIILayout::measureStructureSizeInLayer(
        std::int16_t layer, 
        const std::string& sName, 
        const transform::Matrix& matrix
    ) const -> std::optional<transform::Rect> 
    {
        const auto& boundarys = this->_structures.at(sName)->boundarys;
        auto rects = std::vector<transform::Rect>{};

        for (auto& boundary : boundarys) 
        {
            if (boundary->layer != layer)
                continue;
            auto rect = transform::Rect {
                dbCoordTotransformCoord(boundary->xy.at(0)),
                dbCoordTotransformCoord(boundary->xy.at(2))
            };
            rects.emplace_back(transform::transformRectangle(rect, matrix));
        }

        return transform::unionRectangles(rects);
    }

    auto GDSIILayout::loadFromFile(const std::filesystem::path& filepath) -> std::unique_ptr<GDSIILayout>
    try
    {
        GDSIIReader reader {filepath};
        auto layout { reader.read() };
        return layout;
    }
    catch (const std::exception& exp) 
    {
        debug::exceptionIn("Load gdsii file", exp.what());
    }

    void GDSIILayout::initialize() 
    {
        this->assignRootStructure();
        this->buildPinsBoundary();
    }

    void GDSIILayout::writeGDSII(const std::filesystem::path& filepath) const
    {
        GDSIIWriter{filepath}.write(*this);
    }

    void GDSIILayout::writeTxt(const std::filesystem::path& filepath) const
    {
        auto str = this->toString();
        auto file = std::ofstream{filepath};
        file.write(str.c_str(), str.size());
    }

    auto GDSIILayout::micronToDatabase(double microns) const -> std::int32_t
    {
        // auto metersPerUsrunit = this->_lib->metersPerDBunit / this->_lib->usrunitsPerDBunit;
        int ratio = 1 / (this->_lib->metersPerDBunit * 1e6);
        auto unitsSize = std::round(microns * ratio);
        return unitsSize;
    }

    auto GDSIILayout::databaseToMicron(std::int32_t units) const -> double 
    {
        int ratio = 1 / (this->_lib->metersPerDBunit * 1e6);
        auto meter = units / double(ratio);
        return meter;
    }

    auto GDSIILayout::micronCoordToDatabaseCoord(const GDSIIMicroCoord& coord) const -> GDSIIDBCoord
    {
        auto [x, y] {coord};
        auto xUnit = this->micronToDatabase(x);
        auto yUnit = this->micronToDatabase(y);
        return GDSIIDBCoord {xUnit, yUnit};
    }

    auto GDSIILayout::databseCoordToMicronCoord(const GDSIIDBCoord& coord) const -> GDSIIMicroCoord
    {
        auto [x, y] = coord;
        return GDSIIMicroCoord {
            this->databaseToMicron(x),
            this->databaseToMicron(y)
        };
    }

    auto GDSIILayout::toString(std::stringstream& ss, std::size_t indent) const -> void
    {
        auto addIndent = [&ss] (std::size_t indent) {
            for (std::size_t i = 0; i < indent; ++i)
                ss << "    ";
        };

        addIndent(indent), ss << "GDSII Layout Object\n";
        addIndent(indent + 1), ss << "version: " << this->_lib->version << '\n';
        addIndent(indent + 1), ss << "library name: " << this->_lib->name << '\n';
        addIndent(indent + 1), ss << "units: (" << this->_lib->usrunitsPerDBunit << ", " << this->_lib->metersPerDBunit << ")\n";
        addIndent(indent + 1), ss << "create date: " << this->_lib->createDate.toString();
        addIndent(indent + 1), ss << "modify date: " << this->_lib->modifyDate.toString();

        for (const auto& s : this->_structures) 
            s.second->toString(ss, indent + 1);
    }

    auto GDSIILayout::toString(std::size_t indent) const -> std::string
    {
        auto ss = std::stringstream{};
        this->toString(ss, indent);
        return ss.str();
    } 

    auto GDSIILayout::assignRootStructure() -> void
    {
        // The name never been used in sref
        auto sNames = std::unordered_set<std::string>{};
        for (const auto& [sName, structure] : this->_structures)
            for (const auto& sref : structure->srefs) 
                sNames.emplace(sref->sName);

        for (const auto& [sName, structure] : this->_structures)
        {
            if (!sNames.contains(sName))
            {
                this->_rootStrcutrue = structure;
                return;
            }
        }

        // Error!
        debug::exception("No root structure found");
    }

    auto GDSIILayout::buildPinsBoundary() -> void
    {
        for (const auto& text : this->_rootStrcutrue->texts) 
        {
            for (const auto& boundary : this->_rootStrcutrue->boundarys)
            {
                if (!(boundary->layer == text->layer || boundary->layer + 100 == text->layer)) 
                    continue;

                auto rect = GDSIIDBRect{boundary->xy.at(0), boundary->xy.at(2)};
                if (rect.contains(text->xy.front()))
                {
                    auto result = this->_pinsInfo.emplace(text->string, std::vector<std::tuple<std::int16_t, GDSIIMicroRect>>{});
                    result.first->second.emplace_back(std::make_tuple(
                        boundary->layer,
                        GDSIIMicroRect { 
                            this->databseCoordToMicronCoord(boundary->xy.at(0)),
                            this->databseCoordToMicronCoord(boundary->xy.at(2)),
                        }
                    ));
                }
            }
        }
    }

}