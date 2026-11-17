#pragma once

#include "../base/module.hh"
#include "circuit/base/circuit.hh"
#include "circuit/base/instance.hh"
#include "global/number/number.hh"
#include <string_view>
#include <vector>

namespace xtaro::circuit
{

    /*
        - Ports: 
            -- sa_en we_en
            
            -- bl0 bl1 ...
            -- br0 br1 ...
        
            -- sel0 sel1 ...
            
            -- din0 din1 ...
            -- dout0 dout1 ...
        
            -- vdd gnd

            (For layout)
            -- colmux_vdd, colmux_gnd
            -- sa_vdd, sa_gnd
            -- we_vdd, we_gnd
    */
    class DataPath : public Module
    {
    public:
        static constexpr std::string_view TYPE = "data_path";
        static constexpr std::string_view SENSE_AMP_ENABLE_NAME = "se_en";
        static constexpr std::string_view WRITE_DRIVER_ENABLE_NAME = "we_en";
        
        static constexpr std::string_view SENSE_AMP_VDD_NAME = "se_vdd";
        static constexpr std::string_view SENSE_AMP_GND_NAME = "se_gnd";
        static constexpr std::string_view WRITE_DRIVER_VDD_NAME = "wd_vdd";
        static constexpr std::string_view WRITE_DRIVER_GND_NAME = "wd_gnd";
        static constexpr std::string_view COLUMN_MUX_VDD_NAME = "colmux_vdd";
        static constexpr std::string_view COLUMN_MUX_GND_NAME = "colmux_gnd";

        static auto BITLINE_NAME(int index) -> std::string_view;
        static auto BITLINE_BAR_NAME(int index) -> std::string_view;
        static auto SELECT_NAME(int index) -> std::string_view;
        static auto DATA_INPUT_NAME(int index) -> std::string_view;
        static auto DATA_OUTPUT_NAME(int index) -> std::string_view;

        /*
            - Construction: (int wordWidth, int columnSelSize);
        */
        struct Arguments : public Circuit::Arguments 
        {
            Arguments(int ww, int css) noexcept : 
                Circuit::Arguments {DataPath::TYPE},
                wordWidth {ww},
                columnSelSize {css}
            {
            }

            virtual ~Arguments() noexcept = default;
            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;
            
            int wordWidth;
            int columnSelSize;
        };
        
    public:
        DataPath(std::string_view name, const DataPath::Arguments& arguments);
        virtual ~DataPath() noexcept override = default;

    private:
        virtual void createPorts() override;
        
        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;

    public:
        auto columnMuxArray() -> Circuit* 
        { return this->_columnMuxArray; }

    private:
        int _wordWidth;
        int _columnSelSize;

        int _columnSize;
        bool _useMux;

        Circuit* _columnMuxArray {nullptr};
        Circuit* _writedriverArray {nullptr};
        Circuit* _senseampArray {nullptr};

        Instance* _columnMuxArrayInst {nullptr};
        Instance* _writedriverArrayInst {nullptr};
        Instance* _senseampArrayInst {nullptr};
    
        Layer  _bitlineLayer;
        Number _bitlineWidth;
        Number _bitlinePitch;

        Layer  _horiLayer;
        Number _horiWidth;
        Number _horiPitch;

        Number _instGap;

        Number _columnMuxArrayInstYOff;
        Number _writedriverArrayInstYOff;
        Number _senseampArrayInstYOff;
        std::vector<Number> _outputLineYOffs {};
    };

}