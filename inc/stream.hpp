#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <thread>

#include "err.hpp"
#include "packet.hpp"

namespace ax
{
    /// @brief Fixed or non-fixed length queue between ports
    class Stream
    {
    public:
        Stream(int max_size = -1):
            m_maxSize(max_size)
        {

        }

        ~Stream() = default;

        int max_size() const { return m_maxSize; }

        int size() const { return m_queue.size(); }

        bool empty() const { return m_queue.empty(); }

        /// @brief push packet to stream, allow timeout
        /// @param packet
        /// @param timeout -1 for blocking push, otherwise wait for timeout milliseconds
        int push(const Packet& packet, int timeout = -1)
        {
            std::lock_guard<std::mutex> lg(m_lock);
            // printf("push to 0x%x\n", this);
            if (m_maxSize < 0)
            {
                m_queue.push(packet);
                return AX_SUCCESS;
            }
            else
            {
                if (m_queue.size() >= m_maxSize)
                {
                    if (timeout > 0)
                    {
                        // non-block
                        int nMillisecs = 0;
                        while (m_queue.size() >= m_maxSize && nMillisecs < timeout)
                        {
                            nMillisecs++;
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        }
                        if (m_queue.size() >= m_maxSize)
                            return AX_ERR_QUEUE_FULL;
                        else
                        {
                            m_queue.push(packet);
                            return AX_SUCCESS;
                        }
                    }
                    else
                    {
                        // blocking wait
                        while (m_queue.size() >= m_maxSize)
                        {
                            std::this_thread::sleep_for(std::chrono::microseconds(100));
                        }
                        m_queue.push(packet);
                        return AX_SUCCESS;
                    }
                }
                else
                {
                    m_queue.push(packet);
                    return AX_SUCCESS;
                }
            }
        }

        int pop(Packet& packet)
        {
            std::lock_guard<std::mutex> lg(m_lock);
            // printf("poo from 0x%x\n", this);
            if (m_queue.empty())
            {
                return AX_ERR_QUEUE_EMPTY;
            }
            else
            {
                // printf("queue size: %d\n", m_queue.size());
                packet = m_queue.front();
                m_queue.pop();
                return AX_SUCCESS;
            }
        }

    private:
        int m_maxSize;
        std::mutex m_lock;
        std::queue<Packet> m_queue;
    };
}
