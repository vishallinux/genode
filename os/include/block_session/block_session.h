/*
 * \brief  Block session interface.
 * \author Stefan Kalkowski
 * \date   2010-07-06
 *
 * A block session corresponds to a block device, that can be used to read
 * or store data. Payload is communicated over the packet-stream interface
 * set up between 'Session_client' and 'Session_server'.
 *
 * Even though the functions 'tx' and 'tx_channel' are specific for the client
 * side of the block session interface, they are part of the abstract 'Session'
 * class to enable the client-side use of the block interface via a pointer to
 * the abstract 'Session' class. This way, we can transparently co-locate the
 * packet-stream server with the client in same program.
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BLOCK_SESSION__BLOCK_SESSION_H_
#define _INCLUDE__BLOCK_SESSION__BLOCK_SESSION_H_

#include <os/packet_stream.h>
#include <packet_stream_tx/packet_stream_tx.h>
#include <session/session.h>

namespace Block {

	/**
	 * Represents an operation request with respect to a block,
	 * the data associated with the 'Packet_descriptor' is either
	 * the data read from or written to the block indicated by
	 * its number.
	 */
	class Packet_descriptor : public ::Packet_descriptor
	{
		public:

			enum Opcode    { READ, WRITE, END };
			enum Alignment { PACKET_ALIGNMENT = 11 };

		private:

			Opcode          _op;           /* requested operation */
			Genode::size_t  _block_number; /* requested block number */
			Genode::size_t  _block_count;  /* number of blocks to transfer */
			unsigned        _success :1;   /* indicates success of operation */

		public:

			/**
			 * Constructor
			 */
			Packet_descriptor(Genode::off_t offset=0, Genode::size_t size=0)
			: ::Packet_descriptor(offset, size),
			  _op(READ), _block_number(0), _block_count(0), _success(false) { }

			/**
			 * Constructor
			 */
			Packet_descriptor(Packet_descriptor p, Opcode op,
			                  Genode::size_t blk_nr, Genode::size_t blk_count = 1)
			: ::Packet_descriptor(p.offset(), p.size()),
			  _op(op), _block_number(blk_nr),
			  _block_count(blk_count), _success(false) { }

			Opcode operation() { return _op; }

			Genode::size_t block_number() { return _block_number; }

			Genode::size_t block_count() { return _block_count; }

			bool succeeded() { return _success; }

			void succeeded(bool b) { _success = b ? 1 : 0; }
	};


	struct Session : public Genode::Session
	{
		enum { TX_QUEUE_SIZE = 256 };


		/**
		 * This class represents supported operations on a block device
		 */
		class Operations
		{
			private:

				unsigned _ops :Packet_descriptor::END; /* bitfield of ops */

			public:

				Operations() : _ops(0) { }

				bool supported(Packet_descriptor::Opcode op) {
					return (_ops & (1 << op)); }

				void set_operation(Packet_descriptor::Opcode op) {
					_ops |= (1 << op); }
		};


		typedef Packet_stream_policy<Block::Packet_descriptor,
		                             TX_QUEUE_SIZE, TX_QUEUE_SIZE,
		                             char> Tx_policy;

		typedef Packet_stream_tx::Channel<Tx_policy> Tx;

		static const char *service_name() { return "Block"; }

		virtual ~Session() { }

		/**
		 * Request information about the metrics of the block device
		 *
		 * \param blk_count will contain total number of blocks
		 * \param blk_size  will contain total size in bytes
		 * \param ops       supported operations
		 */
		virtual void info(Genode::size_t *blk_count,
		                  Genode::size_t *blk_size,
		                  Operations     *ops) = 0;

		/**
		 * Request packet-transmission channel
		 */
		virtual Tx *tx_channel() { return 0; }

		/**
		 * Request client-side packet-stream interface of tx channel
		 */
		virtual Tx::Source *tx() { return 0; }


		/*******************
		 ** RPC interface **
		 *******************/

		GENODE_RPC(Rpc_info, void, info, Genode::size_t *, Genode::size_t *, Operations *);
		GENODE_RPC(Rpc_tx_cap, Genode::Capability<Tx>, _tx_cap);
		GENODE_RPC_INTERFACE(Rpc_info, Rpc_tx_cap);
	};
}

#endif /* _INCLUDE__BLOCK_SESSION__BLOCK_SESSION_H_ */
