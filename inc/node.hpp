#pragma once

#include "json/json.h"

#include "port.hpp"
#include "string_utils.hpp"

namespace ax
{
    typedef std::shared_ptr<InputPort>  InputPortPtr;
    typedef std::shared_ptr<OutputPort> OutputPortPtr;

    class Node
    {
    public:        
        Node():
            m_isRunning(false)
        { }

        ~Node() = default;

        Node(const std::string& name):
            m_name(name),
            m_isRunning(false)
        { }

        const char* name() const { return m_name.c_str(); }

        void SetRunning() { m_isRunning = true; }

        virtual int Init(const Json::Value& config) = 0;

        virtual int Run() = 0;

        virtual void Stop() { m_isRunning = false; }

        int GetInputPortNum() const { return m_inputPorts.size(); }
        int GetOutputPortNum() const { return m_outputPorts.size(); }

        InputPortPtr GetInputPort(size_t index) 
        {
            if (index >= m_inputPorts.size()) {
                return nullptr;
            }
            return m_inputPorts[index];
        }

        OutputPortPtr GetOutputPort(size_t index)
        {
            if (index >= m_outputPorts.size()) {
                return nullptr;
            }
            return m_outputPorts[index];
        }

        InputPortPtr FindInputPort(const std::string& name)
        {
            for (const auto& p : m_inputPorts)
            {
                if (p->name() == name)
                    return p;
            }
            return nullptr;
        }

        OutputPortPtr FindOutputPort(const std::string& name)
        {
            for (const auto& p : m_outputPorts)
            {
                if (p->name() == name)
                    return p;
            }
            return nullptr;
        }

        bool AddInputPort(const std::string& port_name)
        {
            if (FindInputPort(port_name) != nullptr)
                return false;
            
            auto iport = std::make_shared<InputPort>(port_name);
            m_inputPorts.push_back(iport);
            return true;
        }

        bool AddOutputPort(const std::string& port_name)
        {
            if (FindOutputPort(port_name) != nullptr)
                return false;

            auto oport = std::make_shared<OutputPort>(port_name);
            m_outputPorts.push_back(oport);
            return true;
        }

        /// @brief automatically connect nodes, whose port name ends with 
        ///     _output and _input, see details for example.
        /// @details Node video_node with output port name "video_output"
        ///     and another node with input port name "video_input", 
        ///     Connect will make stream betweeen "video_input" port and
        ///     "video_output" port.
        /// @param  other   another node
        /// @return num of successfully connected ports.
        int Connect(std::shared_ptr<Node> other)
        {
            if (!other)
                return 0;

            int succ_num = 0;
            for (auto& oport : m_outputPorts)
            {
                if (!utils::ends_with(oport->name(), "_output"))
                    continue;

                std::string sub_oport_name = oport->name().substr(0, oport->name().find("_output"));
                
                for (int i = 0; i < other->GetInputPortNum(); i++)
                {
                    auto iport = other->GetInputPort(i);
                    if (!utils::ends_with(iport->name(), "_input"))
                        continue;

                    std::string sub_iport_name = iport->name().substr(0, iport->name().find("_input"));
                    if (sub_oport_name == sub_iport_name)
                    {
                        oport->connect(iport);
                        succ_num++;
                    }
                }
            }
            
            return succ_num;
        }

        /// @brief  Connect node by port name
        /// @param oport_name 
        /// @param other 
        /// @param iport_name 
        /// @return 
        int Connect(const std::string& oport_name, std::shared_ptr<Node> other, const std::string& iport_name)
        {
            OutputPortPtr oport = FindOutputPort(oport_name);
            if (!oport)     return 0;

            InputPortPtr iport = other->FindInputPort(iport_name);
            if (!iport)     return 0;

            oport->connect(iport);
            return 1;
        }

    protected:
        std::string m_name;
        std::vector<InputPortPtr> m_inputPorts;
        std::vector<OutputPortPtr> m_outputPorts;
        bool m_isRunning;
    };
} // namespace ppl
