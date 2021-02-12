/*   -*- buffer-read-only: t -*- vi: set ro:
 *
 *  DO NOT EDIT THIS FILE   (client-fsm.h)
 *
 *  It has been AutoGen-ed
 *  From the definitions    client.def
 *  and the template file   fsm
 *
 *  Automated Finite State Machine
 *
 *  Copyright (C) 1992-2018 Bruce Korb - all rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Bruce Korb'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * AutoFSM IS PROVIDED BY Bruce Korb ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bruce Korb OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  This file enumerates the states and transition events for a FSM.
 *
 *  te_client_state
 *      The available states.  FSS_INIT is always defined to be zero
 *      and FSS_INVALID and FSS_DONE are always made the last entries.
 *
 *  te_client_event
 *      The transition events.  These enumerate the event values used
 *      to select the next state from the current state.
 *      CLIENT_EV_INVALID is always defined at the end.
 */
#ifndef AUTOFSM_CLIENT_FSM_H_GUARD
#define AUTOFSM_CLIENT_FSM_H_GUARD 1
/**
 *  Finite State machine States
 *
 *  Count of non-terminal states.  The generated states INVALID and DONE
 *  are terminal, but INIT is not  :-).
 */
#define CLIENT_STATE_CT  15
typedef enum {
    CLIENT_ST_INIT,
    CLIENT_ST_WAIT_HELLO_OR_EHLO,
    CLIENT_ST_HELLO_RECEIVED,
    CLIENT_ST_EHLO_RECEIVED,
    CLIENT_ST_CLIENT_INITED,
    CLIENT_ST_MAIL_RECEIVED,
    CLIENT_ST_EXPECTED_RCPT_OR_DATA,
    CLIENT_ST_RCPT_RECEIVED,
    CLIENT_ST_DATA_RECEIVED,
    CLIENT_ST_EXPECTED_MSG_TEXT_OR_END_MSG,
    CLIENT_ST_MSG_RECEIVING,
    CLIENT_ST_END_DATA_RECEIVED,
    CLIENT_ST_VERIFY_RECEIVED,
    CLIENT_ST_QUIT_RECEIVED,
    CLIENT_ST_UNKNOWN_CMD_RECEIVED,
    CLIENT_ST_INVALID,
    CLIENT_ST_DONE
} te_client_state;

/**
 *  Finite State machine transition Events.
 *
 *  Count of the valid transition events
 */
#define CLIENT_EVENT_CT 14
typedef enum {
    CLIENT_EV_TIMEOUT,  CLIENT_EV_SHUTDOWN, CLIENT_EV_RESPONSE,
    CLIENT_EV_EHLO,     CLIENT_EV_HELO,     CLIENT_EV_MAIL,
    CLIENT_EV_RCPT,     CLIENT_EV_DATA,     CLIENT_EV_MSG_TEXT,
    CLIENT_EV_END_DATA, CLIENT_EV_RSET,     CLIENT_EV_QUIT,
    CLIENT_EV_VERIFY,   CLIENT_EV_UNKNOWN,  CLIENT_EV_INVALID
} te_client_event;

/**
 *  Step the FSM.  Returns the resulting state.  If the current state is
 *  CLIENT_ST_DONE or CLIENT_ST_INVALID, it resets to
 *  CLIENT_ST_INIT and returns CLIENT_ST_INIT.
 */
extern te_client_state
client_step(
    te_client_state client_state,
    te_client_event trans_evt,
    void *client,
    void *match_info );

#endif /* AUTOFSM_CLIENT_FSM_H_GUARD */
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of client-fsm.h */
