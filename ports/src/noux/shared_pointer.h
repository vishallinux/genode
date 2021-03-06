/*
 * \brief  Reference-counting smart pointer
 * \author Norman Feske
 * \date   2011-02-17
 */

/*
 * Copyright (C) 2011-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _NOUX__SHARED_POINTER_PTR_H_
#define _NOUX__SHARED_POINTER_PTR_H_

/* Genode includes */
#include <base/printf.h>
#include <base/lock.h>
#include <base/allocator.h>
#include <base/printf.h>

/* Noux includes */
#include <noux_session/sysio.h>
#include <pwd.h>

namespace Noux {

	class Shared_pointer_base;

	class Reference_counter
	{
		private:

			Genode::Lock _lock;
			long         _value;

			friend class Shared_pointer_base;

			void _inc_ref_count()
			{
				Genode::Lock::Guard guard(_lock);
				_value++;
			}

			/**
			 * \return reference counter after decrement
			 */
			long _dec_ref_count()
			{
				Genode::Lock::Guard guard(_lock);
				return --_value;
			}

		public:

			Reference_counter() : _value(0) { }
	};


	class Shared_pointer_base
	{
		protected:

			Reference_counter *_ref_counter;

			Shared_pointer_base(Reference_counter *ref_counter)
			: _ref_counter(ref_counter) { }

			void _inc_ref_count() {
				if (_ref_counter) _ref_counter->_inc_ref_count(); }

			bool _dec_ref_count() {
				return _ref_counter && (_ref_counter->_dec_ref_count() == 0); }

			long count() const { return _ref_counter ? _ref_counter->_value : -99; }
	};


	template <typename T>
	class Shared_pointer : public Shared_pointer_base
	{
		private:

			T                 *_ptr;
			Genode::Allocator *_alloc;

			void _dec_ref_count()
			{
				if (Shared_pointer_base::_dec_ref_count()) {

					if (0)
						PINF("ref count for %p reached zero -> delete object", _ptr);

					Genode::destroy(_alloc, _ptr);
					_ptr         = 0;
					_alloc       = 0;
					_ref_counter = 0;
				}
			}

		public:

			Shared_pointer() : Shared_pointer_base(0), _ptr(0), _alloc(0) { }

			Shared_pointer(T *ptr, Genode::Allocator *alloc)
			: Shared_pointer_base(ptr), _ptr(ptr), _alloc(alloc)
			{
				_inc_ref_count();
			}

			Shared_pointer(Shared_pointer const & from)
			:
				Shared_pointer_base(from._ref_counter),
				_ptr(from._ptr), _alloc(from._alloc)
			{
				_inc_ref_count();
			}

			Shared_pointer & operator=(const Shared_pointer& from)
			{
				/* check for self assignment */
				if (_ptr == from._ptr)
					return *this;

				/* forget about original pointed-to object */
				_dec_ref_count();

				_ref_counter = from._ref_counter;
				_ptr         = from._ptr;
				_alloc       = from._alloc;

				/* account for newly assigned pointed-to object */
				_inc_ref_count();

				return *this;
			}

			~Shared_pointer()
			{
				_dec_ref_count();
			}

			T *      operator -> ()       { return _ptr; }
			T const* operator -> () const { return _ptr; }

			operator bool () const { return _ptr != 0; }
	};
}

#endif /* _NOUX__SHARED_POINTER_H_ */
