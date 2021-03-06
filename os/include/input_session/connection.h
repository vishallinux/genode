/*
 * \brief  Connection to input service
 * \author Norman Feske
 * \date   2008-08-22
 */

/*
 * Copyright (C) 2008-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__INPUT_SESSION__CONNECTION_H_
#define _INCLUDE__INPUT_SESSION__CONNECTION_H_

#include <input_session/client.h>
#include <base/connection.h>

namespace Input {

	struct Connection : Genode::Connection<Session>, Session_client
	{
		Connection()
		:
			Genode::Connection<Session>(session("ram_quota=16K")),
			Session_client(cap())
		{ }
	};
}

#endif /* _INCLUDE__INPUT_SESSION__CONNECTION_H_ */
