#pragma once

#include "../base/module.hh"
#include "utility/misc/math.hh"
#include <optional>

namespace xtaro::circuit
{

    /*
        - Ports (No mbist): 
            -- clk csb we
            -- addr0 addr1 ...
            -- din0 din1 ...
            -- dout0 dout1 ...
            -- vdd gnd

        - Ports (With mbist)
            -- clk 
            -- mbist
            -- csb we
            -- addr0 addr1 ...
            -- din0 din1 ...
            -- bist_csb bist_we
            -- bist_addr0 bist_addr1 ...
            -- bist_din0 bist_din1 ...
            -- dout0 dout1 ...
            -- vdd gnd
    */
    class SRAM : public Module
    {
        enum
        {
            MAX_COL_ADDRESS = 3,
            MAX_CORE_ADDRESS = 2,
        };

        static auto asOneCore(int addrWdith, int wordWidth) -> std::optional<int>;
        static auto addressDistribution(int addrWdith, int wordWidth) -> std::tuple<int, int, int>;

    public:
        static constexpr std::string_view TYPE = "sram";

        /*
            - Construction: (int addressWidth, int wordWidth, bool mbist);
        */
        struct Arguments : public Circuit::Arguments
        {
            Arguments(int aw, int ww, bool m) :
                Circuit::Arguments {SRAM::TYPE},
                addressWidth{aw}, 
                wordWidth{ww},
                mbist{m} {}

            virtual ~Arguments() noexcept override {}

            virtual auto toString() const -> std::string override;
            virtual auto toCircuit(std::string_view circuitName) const -> std::unique_ptr<Circuit> override;

            int addressWidth;
            int wordWidth;
            bool mbist;
        };

    public:
        SRAM(std::string_view name, const SRAM::Arguments& arguments);
        virtual ~SRAM() noexcept override = default;

    private:
        virtual void createPorts() override;

        virtual void createCircuits() override;
        virtual void createInstances() override;

        virtual void calculateLayoutConstants() override;
        virtual void placeInstances() override;
        virtual void routeInstances() override;
        virtual void createLayoutPins() override;
        
    public:
        int addressWidth() const noexcept
        { return this->_addressWidth; }

        int wordWidth() const noexcept
        { return this->_wordWidth; }

        bool mbist() const noexcept
        { return this->_mbist; }

        auto wordSize() const -> int
        { return util::power(2, this->addressWidth()); }

    private:
        int  _addressWidth;
        int  _wordWidth;
        bool _mbist;

        int _coreAddressWidth;
        int _rowAddressWidth;
        int _colAddressWidth;

        int _coreSize;
        int _coreRowSize;
        int _coreColSize;

        int _rowSelSize;
        int _colSelSize;

        bool _multipleCore;

        Circuit* _inputDffs;
        Circuit* _mbistSelector;
        Circuit* _sramCoreSelector;
        Circuit* _sramCore;
        Circuit* _addrDecoder;
    };

}