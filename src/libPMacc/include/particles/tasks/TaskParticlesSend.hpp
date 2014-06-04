/**
 * Copyright 2013-2014 Rene Widera
 *
 * This file is part of libPMacc.
 *
 * libPMacc is free software: you can redistribute it and/or modify
 * it under the terms of of either the GNU General Public License or
 * the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * libPMacc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License and the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and the GNU Lesser General Public License along with libPMacc.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Environment.hpp"
#include "eventSystem/EventSystem.hpp"
#include "traits/NumberOfExchanges.hpp"

namespace PMacc
{

template<class ParBase>
class TaskParticlesSend : public MPITask
{
public:

    enum
    {
        Dim = ParBase::Dim,
        Exchanges = traits::NumberOfExchanges<Dim>::value
    };

    TaskParticlesSend(ParBase &parBase) :
    parBase(parBase),
    state(Constructor)
    {
    }

    virtual void init()
    {
        state = Init;
        EventTask serialEvent = __getTransactionEvent();

        for (int i = 1; i < Exchanges; ++i)
        {
            if (parBase.getParticlesBuffer().hasSendExchange(i))
            {
                __startAtomicTransaction(serialEvent);
                Environment<>::get().ParticleFactory().createTaskSendParticlesExchange(parBase, i);
                tmpEvent += __endTransaction();
            }
            else
            {
                __startAtomicTransaction(serialEvent);
                parBase.deleteGuardParticles(i);
                tmpEvent += __endTransaction();
            }
        }

        state = WaitForSend;
    }

    bool executeIntern()
    {
        switch (state)
        {
        case Init:
            break;
        case WaitForSend:
            return NULL == Environment<>::get().Manager().getITaskIfNotFinished(tmpEvent.getTaskId());
        default:
            return false;
        }

        return false;
    }

    virtual ~TaskParticlesSend()
    {
        notify(this->myId, RECVFINISHED, NULL);
    }

    void event(id_t, EventType, IEventData*)
    {
    }

    std::string toString()
    {
        return "TaskParticlesSend";
    }

private:

    enum state_t
    {
        Constructor,
        Init,
        WaitForSend

    };


    ParBase& parBase;
    state_t state;
    EventTask tmpEvent;
};

} //namespace PMacc
