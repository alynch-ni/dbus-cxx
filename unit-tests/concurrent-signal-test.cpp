// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 *                                                                         *
 *   The dbus-cxx library is free software; you can redistribute it and/or *
 *   modify it under the terms of the GNU General Public License           *
 *   version 3 as published by the Free Software Foundation.               *
 *                                                                         *
 *   The dbus-cxx library is distributed in the hope that it will be       *
 *   useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU   *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this software. If not see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#include <dbus-cxx.h>
#include <iostream>
#include <thread>

static std::shared_ptr<DBus::Dispatcher> dispatch;
static std::shared_ptr<DBus::Connection> conn;

// This test does the following:
// Emit 2 signals from A
// B sends response signals for both initial signals
// Verify that there is no deadlock
int main( int argc, char** argv ) {
    int retval = 0;

    if( argc < 1 ) {
        return 1;
    }

    DBus::set_logging_function( DBus::log_std_err );
    DBus::set_log_level( SL_TRACE );

    dispatch = DBus::StandaloneDispatcher::create();
    conn = dispatch->create_connection( DBus::BusType::SESSION );

    if( argv[ 1 ][ 0 ] == 'A' ) {
        conn->request_name( "dbuscxx.test.sideA" );

        std::shared_ptr<DBus::Object> obj =
            conn->create_object( "/atest", DBus::ThreadForCalling::DispatcherThread );

        std::shared_ptr<DBus::Signal<void(std::string)>> request1 =
                obj->create_signal<void(std::string)>( "dbuscxx.interface", "request1_signal" );
        std::shared_ptr<DBus::Signal<void(std::string)>> request2 =
            obj->create_signal<void(std::string)>("dbuscxx.interface", "request2_signal");

        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

        request1->emit( "foobar" );
        request2->emit("foobar");
        conn->flush();

        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    } else if( argv[ 1 ][ 0 ] == 'B' ) {
        bool req1_received = false;
        bool req2_received = false;
        conn->request_name( "dbuscxx.test.sideB" );

        std::shared_ptr<DBus::Object> obj =
            conn->create_object("/btest", DBus::ThreadForCalling::DispatcherThread);

        std::shared_ptr<DBus::Signal<void(std::string)>> response1 =
            obj->create_signal<void(std::string)>("dbuscxx.interface", "response1_signal");
        std::shared_ptr<DBus::Signal<void(std::string)>> response2 =
            obj->create_signal<void(std::string)>("dbuscxx.interface", "response2_signal");

        std::shared_ptr<DBus::SignalProxy<void(std::string)>> sig1proxy =
                conn->create_free_signal_proxy<void(std::string)>(
                    DBus::MatchRuleBuilder::create()
                    .set_path("/atest")
                    .set_interface("dbuscxx.interface")
                    .set_member("request1_signal")
                    .as_signal_match(),
                    DBus::ThreadForCalling::DispatcherThread);

        sig1proxy->connect([response1, &req1_received](std::string value) {
            response1->emit(value);
            req1_received = true;
            });

        std::shared_ptr<DBus::SignalProxy<void(std::string)>> sig2proxy =
            conn->create_free_signal_proxy<void(std::string)>(
                DBus::MatchRuleBuilder::create()
                .set_path("/atest")
                .set_interface("dbuscxx.interface")
                .set_member("request2_signal")
                .as_signal_match(),
                DBus::ThreadForCalling::DispatcherThread);

        sig2proxy->connect([response2, &req2_received](std::string value) {
            response2->emit(value);
            req2_received = true;
            });

        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

        if( !req1_received || !req2_received ){
            retval = 1;
        }
    }

    return retval;
}
