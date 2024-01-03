#pragma once

#include "err.hpp"
#include "stream.hpp"
#include "packet.hpp"

#include <memory>
#include <vector>

namespace ax
{
    class Port
    {
    public:
        Port() = default;
        Port(const std::string& port_name):
            m_portName(port_name)
            { }

        std::string name() const {
            return m_portName;
        }

        void set_name(const std::string& port_name) {
            m_portName = port_name;
        }

    protected:
        std::string m_portName;
    };

    class InputPort : public Port
    {
        std::shared_ptr<Stream> m_stream;

    public:
        InputPort():
            m_stream(nullptr)
        { }

        InputPort(const std::string& port_name):
            Port(port_name),
            m_stream(nullptr)
        { }

        ~InputPort() = default;

        int recv(Packet& packet)
        {
            if (!has_stream())
            {
                return AX_ERR_NULL_PTR;
            }
                
            return m_stream->pop(packet);
        }

        bool set_stream(const std::shared_ptr<Stream>& stream) 
        { 
            if (m_stream != nullptr)
                return false;

            m_stream = stream; 
            return true;
        }

        bool has_stream() const
        {
            return m_stream != nullptr;
        }
    };

    class OutputPort : public Port
    {
        std::vector<std::shared_ptr<Stream>> m_streams;

    public:
        OutputPort() = default;
        OutputPort(const std::string& port_name):
            Port(port_name)
        { }

        ~OutputPort() = default;

        int send(const Packet& packet)
        {
            if (!packet.isValid())
                return AX_ERR_ILLEGAL_PARAM;

            int ret = AX_SUCCESS;
            if (!has_stream())
            {
                return -1;
            }

            for (const auto& s : m_streams)
            {
                ret = s->push(packet);
                if (ret != AX_SUCCESS)
                    return ret;
            }
            return ret;
        }

        void add_stream(const std::shared_ptr<Stream>& stream)
        {
            m_streams.push_back(stream);
        }

        bool has_stream() const
        {
            return !m_streams.empty();
        }

        void connect(InputPort& iport)
        {
            if (iport.has_stream())
            {
                return;
            }

            auto new_s = std::make_shared<Stream>();
            iport.set_stream(new_s);
            add_stream(new_s);
        }

        void connect(std::shared_ptr<InputPort> iport)
        {
            return connect(*iport);
        }
    };
}