/***************************************************************************
  tag: Peter Soetens  Thu Oct 10 16:16:56 CEST 2002  AnalogOutInterface.hpp 

                        AnalogOutInterface.hpp -  description
                           -------------------
    begin                : Thu October 10 2002
    copyright            : (C) 2002 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be
 
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
 

#ifndef ANALOGOUTINTERFACE_HPP
#define ANALOGOUTINTERFACE_HPP

#include "NameServer.hpp"
#include "NameServerRegistrator.hpp"

namespace StoneHead
{

    /**
     * An interface for writing analog output, like
     * for addressing a whole subdevice in comedi
     * 
     * Measurement Unit (MU) : Unit of what is actually read on the analog channel (e.g. Volt)
     * 
     */
    template < class T>
    class AnalogOutInterface
                : private NameServerRegistrator<AnalogOutInterface<T>*, std::string>
    {
        public:
            /**
             * Create a not nameserved AnalogOutInterface instance.
             */
            AnalogOutInterface()
            {}

            /**
             * Create a nameserved AnalogOutInterface. When <name> is not "" and
             * unique, it can be retrieved using the AnalogOutInterface::nameserver.
             */
            AnalogOutInterface( const std::string& name ) : NameServerRegistrator<AnalogOutInterface<T>*, std::string>( nameserver, name, this )
            {}

            virtual ~AnalogOutInterface()
            {}

            /**
             * Write <value> to channel <chan>
             */
            virtual void write( unsigned int chan, T value ) = 0;

            /**
             * Returns the binary range (e.g. 12bits DA -> 4096)
             */
            virtual T binaryRange() const = 0;

            /**
             * Returns the binary lowest value.
             */
            virtual T binaryLowest() const = 0;

            /**
             * Returns the binary highest value
             */
            virtual T binaryHighest() const = 0;

            /**
             * Returns the lowest measurable input expressed in MU's
             */
            virtual double lowest() const = 0;

            /**
             * Returns the highest measurable input expressed in MU's
             */
            virtual double highest() const = 0;

            /**
             * Resolution is expressed in bits / MU
             */
            virtual double resolution() const = 0;
            
            /**
             * Returns the total number of channels.
             */
            virtual unsigned int nbOfChannels() const = 0;

            /**
             * The NameServer for this interface.
             * @see NameServer
             */
            static NameServer<AnalogOutInterface<T> *> nameserver;

    };

    template <class T>
    NameServer<AnalogOutInterface<T> *> AnalogOutInterface<T>::nameserver;
}

#endif
