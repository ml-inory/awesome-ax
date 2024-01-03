#pragma once

#include <memory>
#include <typeindex>
#include <typeinfo>

namespace ax
{
    /// @brief Type erasure data in stream
    class Packet
    {
        struct PacketConcept {
            virtual ~PacketConcept() {}
            virtual const std::type_info& Type() const = 0;
        };

        template< typename _Ty > struct PacketModel : PacketConcept {
            PacketModel( const _Ty& t ) : pack( t ) {}
            virtual ~PacketModel() {}
            _Ty& get() { return pack; }

            private:
                _Ty pack;
                const std::type_info& Type() const { return typeid(_Ty); }
        };

        std::shared_ptr<PacketConcept> pack;
        bool m_isValid;

    public:
        template< typename _Ty > Packet( const _Ty& _pack ) :
            pack( new PacketModel<_Ty>( _pack ) ),
            m_isValid(true)
        { 
  
        }

        Packet():
            m_isValid(false) { }

        ~Packet()
        {

        }
        
        Packet& operator = (const Packet& other)
        {
            if (&other == this)
                return *this;
            
            pack.reset();
            pack = other.pack;
            m_isValid = other.m_isValid;
            return *this;
        }

        Packet(const Packet& other)
        {
            pack = other.pack;
            m_isValid = other.m_isValid;
        }

        bool isValid() const { return m_isValid; }

        template <typename T>
        bool isType() const 
        { 
            return pack->Type() == typeid(T); 
        }

        template <typename T>
        T& get() const
        {
            return std::dynamic_pointer_cast<PacketModel<T>>(pack)->get();
        }
    };
}
