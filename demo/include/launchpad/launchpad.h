/*
 * \brief  Launchpad interface
 * \author Norman Feske
 * \date   2006-09-01
 *
 * This class is a convenience-wrapper for starting and killing
 * child processes. To support the Launchpad application, is
 * also provides an interface to glues the child handling and
 * the GUI together.
 */

/*
 * Copyright (C) 2006-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__LAUNCHPAD__LAUNCHPAD_H_
#define _INCLUDE__LAUNCHPAD__LAUNCHPAD_H_

#include <base/allocator.h>
#include <base/service.h>
#include <base/printf.h>
#include <base/lock.h>
#include <cap_session/connection.h>
#include <timer_session/timer_session.h>

#include <init/child.h>

class Launchpad_child_policy : public Init::Traditional_child_policy
{
	public:

		Launchpad_child_policy(const char                  *name,
		                       Genode::Server              *server,
		                       Genode::Service_registry    *parent_services,
		                       Genode::Service_registry    *child_services,
		                       Genode::Dataspace_capability config_ds,
		                       Genode::Dataspace_capability binary_ds,
		                       Genode::Rpc_entrypoint      *parent_entrypoint)
		:
			Init::Traditional_child_policy(name, server, parent_services,
			                               child_services, config_ds,
			                               binary_ds, 0, 0, parent_entrypoint)
		{ }

		Genode::Service *resolve_session_request(const char *service_name,
		                                         const char *args)
		{
			Genode::Service *service;

			/* check for config file request */
			if ((service = _config_policy.resolve_session_request(service_name, args)))
				return service;

			/* check for binary file request */
			if ((service = _binary_policy.resolve_session_request(service_name, args)))
				return service;

			/* if service is provided by one of our children, use it */
			if ((service = _child_services->find(service_name)))
				return service;

			/*
			 * Handle special case of the demo scenario when the user uses
			 * a nested Launchad for starting another Nitpicker instance
			 * before starting Liquid_fb. In this case, we do not want to
			 * delegate Nitpicker's session requests to the parent. The
			 * parent may be a launchpad again trying to apply the same
			 * policy. This instance will recognise that the session is not
			 * available at init (because the actual input and framebuffer
			 * drivers are singletons, and would therefore block for
			 * Framebuffer or Input to become available at one of its own
			 * children. The launchpad with which the user originally
			 * interacted, however, would block at the parent interface
			 * until this condition gets satisfied.
			 */
			if (Genode::strcmp(service_name, "Input") != 0
			 && Genode::strcmp(service_name, "Framebuffer") != 0
			 && (service = _parent_services->find(service_name)))
				return service;

			/* wait for the service to become available */
			Genode::Client client;
			return _child_services->wait_for_service(service_name,
			                                         &client, name());
		}
};


class Launchpad;


class Launchpad_child : public Genode::List<Launchpad_child>::Element
{
	private:

		Launchpad *_launchpad;

		Genode::Rom_session_capability _rom;
		Genode::Ram_session_capability _ram;
		Genode::Cpu_session_capability _cpu;
		Genode::Rm_session_capability  _rm;
		Genode::Server                 _server;

		/*
		 * Entry point used for serving the parent interface and the
		 * locally provided ROM sessions for the 'config' and 'binary'
		 * files.
		 */
		enum { ENTRYPOINT_STACK_SIZE = 12*1024 };
		Genode::Rpc_entrypoint _entrypoint;

		Launchpad_child_policy _policy;
		Genode::Child          _child;

	public:

			Launchpad_child(const char                    *name,
			                Genode::Dataspace_capability   elf_ds,
			                Genode::Ram_session_capability ram,
			                Genode::Cpu_session_capability cpu,
			                Genode::Rm_session_capability  rm,
			                Genode::Rom_session_capability rom,
			                Genode::Cap_session           *cap_session,
			                Genode::Service_registry      *parent_services,
			                Genode::Service_registry      *child_services,
			                Genode::Dataspace_capability   config_ds,
			                Launchpad                     *launchpad)
			:
				_launchpad(launchpad),
				_rom(rom), _ram(ram), _cpu(cpu), _rm(rm), _server(_ram),
				_entrypoint(cap_session, ENTRYPOINT_STACK_SIZE, name, false),
				_policy(name, &_server, parent_services, child_services,
				        config_ds, elf_ds, &_entrypoint),
				_child(elf_ds, ram, cpu, rm, &_entrypoint, &_policy) {
				_entrypoint.activate(); }

			Genode::Rom_session_capability rom_session_cap() { return _rom; }
			Genode::Ram_session_capability ram_session_cap() { return _ram; }
			Genode::Cpu_session_capability cpu_session_cap() { return _cpu; }
			Genode::Rm_session_capability  rm_session_cap()  { return _rm;  }

			const char *name() const { return _policy.name(); }

			const Genode::Server *server() const { return &_server; }

			Genode::Allocator *heap() { return _child.heap(); }

			void revoke_server(const Genode::Server *server) {
				_child.revoke_server(server); }
};


class Launchpad
{
	private:

		unsigned long _initial_quota;

		Genode::Service_registry _parent_services;
		Genode::Service_registry _child_services;

		Genode::Lock                  _children_lock;
		Genode::List<Launchpad_child> _children;

		bool _child_name_exists(const char *name);
		void _get_unique_child_name(const char *filename, char *dst, int dst_len);

		Genode::Sliced_heap _sliced_heap;

		/* cap session for allocating capabilities for parent interfaces */
		Genode::Cap_connection _cap_session;

	protected:

		int _ypos;

	public:

		Genode::Lock gui_lock;

		Launchpad(unsigned long initial_quota);

		unsigned long initial_quota() { return _initial_quota; }

		virtual ~Launchpad() { }


		/*************************
		 ** Configuring the GUI **
		 *************************/

		virtual void quota(unsigned long quota) { }

		virtual void add_launcher(const char *filename,
		                          unsigned long default_quota,
		                          Genode::Dataspace_capability config_ds) { }

		virtual void add_child(const char *unique_name,
		                       unsigned long quota,
		                       Launchpad_child *launchpad_child,
		                       Genode::Allocator *alloc) { }

		virtual void remove_child(const char *name,
		                          Genode::Allocator *alloc) { }

//		virtual void child_quota(const char *name, unsigned long quota) { }

		Launchpad_child *start_child(const char *prg_name, unsigned long quota,
		                             Genode::Dataspace_capability config_ds);

		/**
		 * Exit child and close all its sessions
		 *
		 * \param timer                     Timer session to use for watchdog
		 *                                  mechanism. When using the default
		 *                                  value, a new timer session gets
		 *                                  created during the 'exit_child'
		 *                                  call.
		 * \param session_close_timeout_ms  Timeout in milliseconds until a an
		 *                                  unresponsive service->close call
		 *                                  gets canceled.
		 */
		void exit_child(Launchpad_child *child,
		                Timer::Session  *timer = 0,
		                int              session_close_timeout_ms = 2000);
};

#endif /* _INCLUDE__LAUNCHPAD__LAUNCHPAD_H_ */
