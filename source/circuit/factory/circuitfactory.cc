#include "./circuitfactory.hh"

#include "../base/circuit.hh"
#include "circuit/factory/argument.hh"
#include "circuit/module/dffcolumn.hh"
#include "circuit/module/dffrow.hh"
#include "circuit/module/replicabitcellarray.hh"
#include "circuit/module/replicatedbitcellarray.hh"
#include "circuit/module/sram1bank.hh"
#include <circuit/module/wordlinederiver.hh>
#include <circuit/module/wordlinederiverarr.hh>
#include <circuit/component/mos.hh>
#include <circuit/libmodule/bitcell.hh>
#include <circuit/module/andarray.hh>
#include <circuit/libmodule/dff.hh>
#include <circuit/libmodule/precharge.hh>
#include <circuit/libmodule/senseamp.hh>
#include <circuit/module/trigate.hh>
#include <circuit/libmodule/writedriver.hh>
#include <circuit/module/bitcellarray.hh>
#include <circuit/libmodule/inv.hh>
#include <circuit/libmodule/nand.hh>
#include <circuit/libmodule/and.hh>
#include <circuit/libmodule/nor.hh>
#include <circuit/module/buffer.hh>
#include <circuit/libmodule/or.hh>
#include <circuit/module/decoder.hh>
#include <circuit/module/rowdecoder.hh>
#include <circuit/module/selector.hh>
#include <circuit/module/mbistselector.hh>
#include <circuit/module/fanoutbuff.hh>
#include <circuit/module/senseamparr.hh>
#include <circuit/module/writedriverarr.hh>
#include <circuit/module/prechargearr.hh>
#include <circuit/module/replicabank.hh>
#include <circuit/module/controllogic.hh>
#include <circuit/module/inputdffs.hh>
#include <circuit/module/sram.hh>
#include <circuit/module/sramcore.hh>
#include <circuit/module/datapath.hh>
#include <circuit/module/prebitcellarray.hh>
#include <circuit/module/sramcoreselector.hh>
#include <circuit/libmodule/columntrigate.hh>
#include <circuit/module/columnmux.hh>
#include <circuit/module/columnmuxarr.hh>
#include <circuit/module/test.hh>

#include <exception>
#include <format>
#include <global/config/config.hh>
#include <global/tech/tech.hh>
#include <optional>
#include <utility/debug/debug.hh>
#include <utility/misc/string.hh>

#include <string>
#include <unordered_map>
#include <string_view>
#include <memory>

namespace xtaro::circuit
{

    // ====================================================================== //
    //                         Public Interface                               //
    // ====================================================================== //


    void CircuitFactory::reset() noexcept
    {
        // Clear circuit pools
        this->_circuitPool.clear();

        // Clear string pools
        this->_stringPool.clear();
    }

    auto CircuitFactory::string(std::string str) -> std::string_view
    {
        auto res = this->_stringPool.emplace(std::move(str));
        return std::string_view {res.first->c_str()};
    }

    auto CircuitFactory::circuit(const Circuit::Arguments& arguments) -> Circuit*
    try
    {
        // Get Circuit name
        auto circuitName = this->getCircuitName(arguments);

        // Circuit exits?
        auto circuitOpt = this->findCircuit(circuitName);
        if (circuitOpt.has_value())
            return circuitOpt.value();

        // Create a new circuit
        auto newCircuit = this->createNewCircuit(circuitName, arguments);
        auto circuit = newCircuit.get();

        // Collect
        this->collectCircuit(circuitName, std::move(newCircuit));

        return circuit;
    }
    catch (const std::exception& err) 
    {
        debug::exceptionIn(
            std::format("Create '{}'", arguments.circuitType()),
            err.what()
        );
    }

    auto CircuitFactory::circuit(
        std::string_view circuitType, 
        const std::unordered_map<std::string, Argument>& arguments) -> Circuit*
    {
        // Build Arguments
        auto cirArguments = this->createCircuitArguments(circuitType, arguments);
        return this->circuit(*cirArguments);
    }

    // ====================================================================== //
    //                            Inner Method                                //
    // ====================================================================== //

    auto CircuitFactory::getCircuitName(const Circuit::Arguments& arguments) -> std::string_view
    {
        std::string argumentsString {arguments.toString()};

        if (argumentsString.empty())
            return this->stringf("{}", arguments.circuitType());
        else {
            return this->stringf("{}_{}", arguments.circuitType(), argumentsString);
        }
    }

    auto CircuitFactory::findCircuit(std::string_view circuitName) const -> std::optional<Circuit*>
    {
        auto res = this->_circuitPool.find(circuitName);
        if (res == this->_circuitPool.end())
            return std::nullopt;
        return {res->second.get()};
    }
 
    template <typename CircuitClass>
    static auto create(std::string_view circuitName, const Circuit::Arguments& arguments)
    {
        using CircuitClassArgument = typename CircuitClass::Arguments;
        return std::make_shared<CircuitClass>(
            circuitName, 
            dynamic_cast<const CircuitClassArgument&>(arguments)
        );
    }

    auto CircuitFactory::createNewCircuit(
        std::string_view circuitName,
        const Circuit::Arguments& arguments
    ) const -> std::unique_ptr<Circuit>
    {
        debug::debugf("Create '{}' circuit: '{}'", arguments.circuitType(), circuitName.data());
        return arguments.toCircuit(circuitName);
    }

    auto getArgument(const std::unordered_map<std::string, Argument>& arguments, const std::string& name) -> const Argument& 
    try 
    {
        return arguments.at(name);
    }
    catch (const std::exception&)
    {
        debug::exceptionfIn("Get argument", "Un exit argument '{}'", name);
    }


    #define REGISTER_ARGUMENT(Type, name)\
    getArgument(arguments, #name).get<Type>()
    
    #define ARGUMENT_BUILDER_FUNC_TEMPPLATE(ArgumentClass, ...)\
    {\
        std::string_view{ ArgumentClass::TYPE },\
        [] (const std::unordered_map<std::string, Argument>& arguments) -> std::unique_ptr<Circuit::Arguments> {\
            return std::make_unique<ArgumentClass::Arguments>(\
                __VA_ARGS__\
            );\
        }\
    }

    using ArgumentBuilderFunc = std::unique_ptr<Circuit::Arguments>(*)(const std::unordered_map<std::string, Argument>&);
    static std::unordered_map<std::string_view, ArgumentBuilderFunc> circuitTypeToArgument {
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(DFF),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(Bitcell),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(SenseAmplifier),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(WriteDriver),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(ColumnTristateGate),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(Precharge),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(TristateGate),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(NAND, 
            REGISTER_ARGUMENT(DriveAbility, da),
            REGISTER_ARGUMENT(int, is)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(AND, 
            REGISTER_ARGUMENT(DriveAbility, da),
            REGISTER_ARGUMENT(int, is)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(ANDArray, 
            REGISTER_ARGUMENT(int, as)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(NOR, 
            REGISTER_ARGUMENT(DriveAbility, da),
            REGISTER_ARGUMENT(int, is)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(OR, 
            REGISTER_ARGUMENT(DriveAbility, da),
            REGISTER_ARGUMENT(int, is)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(INV, 
            REGISTER_ARGUMENT(DriveAbility, da)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(Decoder, 
            REGISTER_ARGUMENT(int, is)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(Selector, 
            REGISTER_ARGUMENT(int, ss)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(BitcellArray, 
            REGISTER_ARGUMENT(int, rs),
            REGISTER_ARGUMENT(int, cs)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(WriteDriver),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(WriteDriverArray, 
            REGISTER_ARGUMENT(int, cs),
            REGISTER_ARGUMENT(int, scs)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(ControlLogic),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(MOS, 
            REGISTER_ARGUMENT(MOSType, mt),
            REGISTER_ARGUMENT(Number, w),
            REGISTER_ARGUMENT(Number, l)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(SRAM, 
            REGISTER_ARGUMENT(int, aw),
            REGISTER_ARGUMENT(int, ww),
            REGISTER_ARGUMENT(bool, mbist)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(PrechargeArray,
            REGISTER_ARGUMENT(int, cs)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(Bitcell),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(Test),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(SenseAmplifier),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(SenseAmplifierArray,
            REGISTER_ARGUMENT(int, cs),
            REGISTER_ARGUMENT(int, scs)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(ColumnTristateGate),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(ColumnMux,
            REGISTER_ARGUMENT(int, ss)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(ColumnMuxArray,
            REGISTER_ARGUMENT(int, ss),
            REGISTER_ARGUMENT(int, ms)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(DataPath,
            REGISTER_ARGUMENT(int, ww),
            REGISTER_ARGUMENT(int, css)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(RowDecoder,
            REGISTER_ARGUMENT(int, as)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(Buffer,
            REGISTER_ARGUMENT(DriveAbility, dc)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(WordlineDriver, 
            REGISTER_ARGUMENT(int, fo)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(WordlineDriverArray, 
            REGISTER_ARGUMENT(int, fo),
            REGISTER_ARGUMENT(int, ws)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(ReplicaBitcellArray,
            REGISTER_ARGUMENT(int, bs)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(ReplicatedBitcellArray,
            REGISTER_ARGUMENT(int, rs),
            REGISTER_ARGUMENT(int, cs)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(SRAM1Bank,
            REGISTER_ARGUMENT(int, aw),
            REGISTER_ARGUMENT(int, caw),
            REGISTER_ARGUMENT(int, ww)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(DFlipFlopColumn,
            REGISTER_ARGUMENT(int, ds)
        ),
        ARGUMENT_BUILDER_FUNC_TEMPPLATE(DFlipFlopRow,
            REGISTER_ARGUMENT(int, ds)
        )
    };

#undef ARGUMENT_BUILDER_FUNC_TEMPPLATE

    auto CircuitFactory::createCircuitArguments(
        std::string_view circuitType,
        const std::unordered_map<std::string, Argument>& arguments
    ) -> std::unique_ptr<Circuit::Arguments>
    {
        auto res {circuitTypeToArgument.find(circuitType)};
        if (res == circuitTypeToArgument.end())
        {
            debug::exceptionfIn("Create circuit", "No support circuit '{}' to test", circuitType);
            return nullptr;
        }
        return res->second(arguments);
    }

    void CircuitFactory::collectCircuit(
        std::string_view circuitName,
        std::unique_ptr<Circuit> circuit
    )
    {
        this->_circuitPool.emplace(circuitName, std::move(circuit));
    }

    CircuitFactory::~CircuitFactory() noexcept
    {
    }
}