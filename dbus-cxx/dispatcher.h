/***************************************************************************
 *   Copyright (C) 2007,2008,2009,2010 by Rick L. Vinyard, Jr.             *
 *   rvinyard@cs.nmsu.edu                                                  *
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
#include <dbus/dbus.h>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <poll.h>
#include "enums.h"

#ifndef DBUSCXX_DISPATCHER_H
#define DBUSCXX_DISPATCHER_H

namespace DBus
{
  class Connection;

  /**
   * Handles multi-threaded dispatching of one or more connections.
   *
   * This class provides a way to handle multiple connections with one set of
   * dispatching threads.
   *
   * This dispatcher creates a separate thread to handle the reading/writing
   * of data to the bus.
   *
   * @ingroup core
   *
   * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
   */
  class Dispatcher
  {
    protected:

      Dispatcher(bool is_running=true);

    public:

      static std::shared_ptr<Dispatcher> create( bool is_running=true );

      virtual ~Dispatcher();
      
      /** @name Managing Connections */
      //@{
      std::shared_ptr<Connection> create_connection( BusType type );

      std::shared_ptr<Connection> create_connection( std::string address );

      bool add_connection( std::shared_ptr<Connection> connection );

      //@}

      bool start();
      
      bool stop();
      
      bool is_running();

    protected:
      
      typedef std::vector<std::shared_ptr<Connection>> Connections;
      Connections m_connections;
      
      volatile bool m_running;
      
      std::thread m_dispatch_thread;
      
      /* socketpair for telling the thread to process data */
      int process_fd[ 2 ];

      /**
       * This is the maximum number of dispatches that will occur for a
       * connection in one iteration of the dispatch thread.
       *
       * If set to 0, a particular connection will continue to dispatch
       * as long as its status remains DISPATCH_DATA_REMAINS.
       */
      unsigned int m_dispatch_loop_limit;
      
      void dispatch_thread_main();
      
      void on_wakeup_main(std::shared_ptr<Connection>);
      
      void on_dispatch_status_changed(DispatchStatus, std::shared_ptr<Connection>);

      void wakeup_thread();

      /**
       * Dispatch all of our connections
       */
      void dispatch_connections();
  };

}

#endif
