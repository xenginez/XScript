#pragma once

#include <array>
#include <atomic>
#include <thread>
#include <limits>
#include <climits>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <algorithm>
#include <type_traits>

namespace x
{
	struct producer_token;
	struct consumer_token;
	template<typename T, typename Traits> class concurrent_queue;

	namespace details
	{
		struct concurrent_queue_producer_typeless_base
		{
			concurrent_queue_producer_typeless_base * next;
			std::atomic<bool> inactive;
			producer_token * token;

			concurrent_queue_producer_typeless_base()
				: next( nullptr ), inactive( false ), token( nullptr )
			{
			}
		};

		template<typename T> static inline bool circular_less_than( T a, T b )
		{
			static_assert( std::is_integral<T>::value && !std::numeric_limits<T>::is_signed, "circular_less_than is intended to be used only with unsigned integer types" );
			return ( a - b ) > ( (T)( 1 ) << (T)( sizeof( T ) * CHAR_BIT - 1 ) );
		}

		template<typename U> static inline char * align_for( char * ptr )
		{
			const std::size_t alignment = std::alignment_of<U>::value;
			return ptr + ( alignment - ( reinterpret_cast<std::uintptr_t>( ptr ) % alignment ) ) % alignment;
		}

		template<typename T> static inline T ceil_to_pow_2( T x )
		{
			static_assert( std::is_integral<T>::value && !std::numeric_limits<T>::is_signed, "ceil_to_pow_2 is intended to be used only with unsigned integer types" );

			--x;
			x |= x >> 1;
			x |= x >> 2;
			x |= x >> 4;
			for ( std::size_t i = 1; i < sizeof( T ); i <<= 1 )
			{
				x |= x >> ( i << 3 );
			}
			++x;
			return x;
		}

		template<typename T> static inline void swap_relaxed( std::atomic<T> & left, std::atomic<T> & right )
		{
			T temp = std::move( left.load( std::memory_order_relaxed ) );
			left.store( std::move( right.load( std::memory_order_relaxed ) ), std::memory_order_relaxed );
			right.store( std::move( temp ), std::memory_order_relaxed );
		}

		template<typename T> static inline T const & nomove( T const & x )
		{
			return x;
		}

		template<bool Enable> struct nomove_if
		{
			template<typename T>
			static inline T const & eval( T const & x )
			{
				return x;
			}
		};

		template<> struct nomove_if<false>
		{
			template<typename U>
			static inline auto eval( U && x ) -> decltype( std::forward<U>( x ) )
			{
				return std::forward<U>( x );
			}
		};

		template<typename It> static inline auto deref_noexcept( It & it ) noexcept -> decltype( *it )
		{
			return *it;
		}

		template<typename T> struct static_is_lock_free_num
		{
			static constexpr int value = 0;
		};
		template<> struct static_is_lock_free_num<signed char>
		{
			static constexpr int value = ATOMIC_CHAR_LOCK_FREE;
		};
		template<> struct static_is_lock_free_num<short>
		{
			static constexpr int value = ATOMIC_SHORT_LOCK_FREE;
		};
		template<> struct static_is_lock_free_num<int>
		{
			static constexpr int value = ATOMIC_INT_LOCK_FREE;
		};
		template<> struct static_is_lock_free_num<long>
		{
			static constexpr int value = ATOMIC_LONG_LOCK_FREE;
		};
		template<> struct static_is_lock_free_num<long long>
		{
			static constexpr int value = ATOMIC_LLONG_LOCK_FREE;
		};
		template<typename T> struct static_is_lock_free : static_is_lock_free_num<typename std::make_signed<T>::type>
		{
		};
		template<> struct static_is_lock_free<bool>
		{
			static constexpr int value = ATOMIC_BOOL_LOCK_FREE;
		};
		template<typename U> struct static_is_lock_free<U *>
		{
			static constexpr int value = ATOMIC_POINTER_LOCK_FREE;
		};
	}

	struct concurrent_queue_default_traits
	{
		typedef std::size_t size_t;
		typedef std::size_t index_t;

		static constexpr size_t BLOCK_SIZE = 32;
		static constexpr size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = 32;
		static constexpr size_t EXPLICIT_INITIAL_INDEX_SIZE = 32;
		static constexpr size_t IMPLICIT_INITIAL_INDEX_SIZE = 32;
		static constexpr size_t INITIAL_IMPLICIT_PRODUCER_HASH_SIZE = 32;
		static constexpr uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = 256;
		static constexpr size_t MAX_SUBQUEUE_SIZE = std::numeric_limits<size_t>::max();

		static inline void * malloc( size_t size )
		{
			return std::malloc( size );
		}
		static inline void free( void * ptr )
		{
			return std::free( ptr );
		}
	};

	struct producer_token
	{
		template<typename T, typename Traits> explicit producer_token( concurrent_queue<T, Traits> & queue );

		producer_token( producer_token && other ) noexcept
			: producer( other.producer )
		{
			other.producer = nullptr;
			if ( producer != nullptr )
			{
				producer->token = this;
			}
		}

		inline producer_token & operator=( producer_token && other ) noexcept
		{
			swap( other );
			return *this;
		}

		void swap( producer_token & other ) noexcept
		{
			std::swap( producer, other.producer );
			if ( producer != nullptr )
			{
				producer->token = this;
			}
			if ( other.producer != nullptr )
			{
				other.producer->token = &other;
			}
		}

		inline bool valid() const
		{
			return producer != nullptr;
		}

		~producer_token()
		{
			if ( producer != nullptr )
			{
				producer->token = nullptr;
				producer->inactive.store( true, std::memory_order_release );
			}
		}

		producer_token( producer_token const & ) = delete;
		producer_token & operator=( producer_token const & ) = delete;

	private:
		template<typename T, typename Traits> friend class concurrent_queue;

	protected:
		details::concurrent_queue_producer_typeless_base * producer;
	};

	struct consumer_token
	{
		template<typename T, typename Traits> explicit consumer_token( concurrent_queue<T, Traits> & q );

		consumer_token( consumer_token && other ) noexcept
			: initialOffset( other.initialOffset ), lastKnownGlobalOffset( other.lastKnownGlobalOffset ), itemsConsumedFromCurrent( other.itemsConsumedFromCurrent ), currentProducer( other.currentProducer ), desiredProducer( other.desiredProducer )
		{
		}

		inline consumer_token & operator=( consumer_token && other ) noexcept
		{
			swap( other );
			return *this;
		}

		void swap( consumer_token & other ) noexcept
		{
			std::swap( initialOffset, other.initialOffset );
			std::swap( lastKnownGlobalOffset, other.lastKnownGlobalOffset );
			std::swap( itemsConsumedFromCurrent, other.itemsConsumedFromCurrent );
			std::swap( currentProducer, other.currentProducer );
			std::swap( desiredProducer, other.desiredProducer );
		}

		consumer_token( consumer_token const & ) = delete;
		consumer_token & operator=( consumer_token const & ) = delete;

	private:
		template<typename T, typename Traits> friend class concurrent_queue;

	private:
		std::uint32_t initialOffset;
		std::uint32_t lastKnownGlobalOffset;
		std::uint32_t itemsConsumedFromCurrent;
		details::concurrent_queue_producer_typeless_base * currentProducer;
		details::concurrent_queue_producer_typeless_base * desiredProducer;
	};

	template<typename T, typename Traits> inline void swap( typename concurrent_queue<T, Traits>::implicit_producer_kvp & a, typename concurrent_queue<T, Traits>::implicit_producer_kvp & b ) noexcept;

	template<typename T, typename Traits = concurrent_queue_default_traits> class concurrent_queue
	{
	public:
		typedef producer_token producer_token_t;
		typedef consumer_token consumer_token_t;

		typedef typename Traits::size_t size_t;
		typedef typename Traits::index_t index_t;

		static constexpr size_t BLOCK_SIZE = static_cast<size_t>( Traits::BLOCK_SIZE );
		static constexpr size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = static_cast<size_t>( Traits::EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD );
		static constexpr size_t EXPLICIT_INITIAL_INDEX_SIZE = static_cast<size_t>( Traits::EXPLICIT_INITIAL_INDEX_SIZE );
		static constexpr size_t IMPLICIT_INITIAL_INDEX_SIZE = static_cast<size_t>( Traits::IMPLICIT_INITIAL_INDEX_SIZE );
		static constexpr size_t INITIAL_IMPLICIT_PRODUCER_HASH_SIZE = static_cast<size_t>( Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE );
		static constexpr uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = static_cast<std::uint32_t>( Traits::EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE );
		static constexpr size_t MAX_SUBQUEUE_SIZE = ( std::numeric_limits<size_t>::max() - static_cast<size_t>( Traits::MAX_SUBQUEUE_SIZE ) < BLOCK_SIZE ) ? std::numeric_limits<size_t>::max() : ( ( static_cast<size_t>( Traits::MAX_SUBQUEUE_SIZE ) + ( BLOCK_SIZE - 1 ) ) / BLOCK_SIZE * BLOCK_SIZE );

		static_assert( !std::numeric_limits<size_t>::is_signed && std::is_integral<size_t>::value, "Traits::size_t must be an unsigned integral type" );
		static_assert( !std::numeric_limits<index_t>::is_signed && std::is_integral<index_t>::value, "Traits::index_t must be an unsigned integral type" );
		static_assert( sizeof( index_t ) >= sizeof( size_t ), "Traits::index_t must be at least as wide as Traits::size_t" );
		static_assert( ( BLOCK_SIZE > 1 ) && !( BLOCK_SIZE & ( BLOCK_SIZE - 1 ) ), "Traits::BLOCK_SIZE must be a power of 2 (and at least 2)" );
		static_assert( ( EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD > 1 ) && !( EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD & ( EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD - 1 ) ), "Traits::EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD must be a power of 2 (and greater than 1)" );
		static_assert( ( EXPLICIT_INITIAL_INDEX_SIZE > 1 ) && !( EXPLICIT_INITIAL_INDEX_SIZE & ( EXPLICIT_INITIAL_INDEX_SIZE - 1 ) ), "Traits::EXPLICIT_INITIAL_INDEX_SIZE must be a power of 2 (and greater than 1)" );
		static_assert( ( IMPLICIT_INITIAL_INDEX_SIZE > 1 ) && !( IMPLICIT_INITIAL_INDEX_SIZE & ( IMPLICIT_INITIAL_INDEX_SIZE - 1 ) ), "Traits::IMPLICIT_INITIAL_INDEX_SIZE must be a power of 2 (and greater than 1)" );
		static_assert( ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 ) || !( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE & ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE - 1 ) ), "Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE must be a power of 2" );
		static_assert( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 || INITIAL_IMPLICIT_PRODUCER_HASH_SIZE >= 1, "Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE must be at least 1 (or 0 to disable implicit enqueueing)" );

	public:
		static bool is_lock_free()
		{
			return
				details::static_is_lock_free<bool>::value == 2 &&
				details::static_is_lock_free<size_t>::value == 2 &&
				details::static_is_lock_free<std::uint32_t>::value == 2 &&
				details::static_is_lock_free<index_t>::value == 2 &&
				details::static_is_lock_free<void *>::value == 2;
		}

	public:
		explicit concurrent_queue( size_t capacity = 6 * BLOCK_SIZE )
			: producerListTail( nullptr ), producerCount( 0 ), initialBlockPoolIndex( 0 ), nextExplicitConsumerId( 0 ), globalExplicitConsumerOffset( 0 )
		{
			implicitProducerHashResizeInProgress.clear( std::memory_order_relaxed );
			populate_initial_implicit_producer_hash();
			populate_initial_block_list( capacity / BLOCK_SIZE + ( ( capacity & ( BLOCK_SIZE - 1 ) ) == 0 ? 0 : 1 ) );
		}
		concurrent_queue( size_t minCapacity, size_t maxExplicitProducers, size_t maxImplicitProducers )
			: producerListTail( nullptr ), producerCount( 0 ), initialBlockPoolIndex( 0 ), nextExplicitConsumerId( 0 ), globalExplicitConsumerOffset( 0 )
		{
			implicitProducerHashResizeInProgress.clear( std::memory_order_relaxed );
			populate_initial_implicit_producer_hash();
			size_t blocks = ( ( ( minCapacity + BLOCK_SIZE - 1 ) / BLOCK_SIZE ) - 1 ) * ( maxExplicitProducers + 1 ) + 2 * ( maxExplicitProducers + maxImplicitProducers );
			populate_initial_block_list( blocks );
		}
		concurrent_queue( concurrent_queue const & ) = delete;
		concurrent_queue & operator=( concurrent_queue const & ) = delete;
		concurrent_queue( concurrent_queue && other ) noexcept
			: producerListTail( other.producerListTail.load( std::memory_order_relaxed ) )
			, producerCount( other.producerCount.load( std::memory_order_relaxed ) )
			, initialBlockPoolIndex( other.initialBlockPoolIndex.load( std::memory_order_relaxed ) )
			, initialBlockPool( other.initialBlockPool )
			, initialBlockPoolSize( other.initialBlockPoolSize )
			, freeList( std::move( other.freeList ) )
			, nextExplicitConsumerId( other.nextExplicitConsumerId.load( std::memory_order_relaxed ) )
			, globalExplicitConsumerOffset( other.globalExplicitConsumerOffset.load( std::memory_order_relaxed ) )
		{
			implicitProducerHashResizeInProgress.clear( std::memory_order_relaxed );
			populate_initial_implicit_producer_hash();
			swap_implicit_producer_hashes( other );

			other.producerListTail.store( nullptr, std::memory_order_relaxed );
			other.producerCount.store( 0, std::memory_order_relaxed );
			other.nextExplicitConsumerId.store( 0, std::memory_order_relaxed );
			other.globalExplicitConsumerOffset.store( 0, std::memory_order_relaxed );

			other.initialBlockPoolIndex.store( 0, std::memory_order_relaxed );
			other.initialBlockPoolSize = 0;
			other.initialBlockPool = nullptr;

			reown_producers();
		}
		inline concurrent_queue & operator=( concurrent_queue && other ) noexcept
		{
			swap( other );

			return *this;
		}
		~concurrent_queue()
		{
			auto ptr = producerListTail.load( std::memory_order_relaxed );
			while ( ptr != nullptr )
			{
				auto next = ptr->next_prod();
				if ( ptr->token != nullptr )
				{
					ptr->token->producer = nullptr;
				}
				destroy( ptr );
				ptr = next;
			}

			if ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE != 0 )
			{
				auto hash = implicitProducerHash.load( std::memory_order_relaxed );
				while ( hash != nullptr )
				{
					auto prev = hash->prev;
					if ( prev != nullptr )
					{
						for ( size_t i = 0; i != hash->capacity; ++i )
						{
							hash->entries[i].~implicit_producer_kvp();
						}
						hash->~implicit_producer_hash();
						(Traits::free)( hash );
					}
					hash = prev;
				}
			}

			auto block = freeList.head_unsafe();
			while ( block != nullptr )
			{
				auto next = block->freeListNext.load( std::memory_order_relaxed );
				if ( block->dynamicallyAllocated )
				{
					destroy( block );
				}
				block = next;
			}

			destroy_array( initialBlockPool, initialBlockPoolSize );
		}

	public:
		inline void swap( concurrent_queue & other ) noexcept
		{
			if ( this == &other )
			{
				return *this;
			}

			details::swap_relaxed( producerListTail, other.producerListTail );
			details::swap_relaxed( producerCount, other.producerCount );
			details::swap_relaxed( initialBlockPoolIndex, other.initialBlockPoolIndex );
			std::swap( initialBlockPool, other.initialBlockPool );
			std::swap( initialBlockPoolSize, other.initialBlockPoolSize );
			freeList.swap( other.freeList );
			details::swap_relaxed( nextExplicitConsumerId, other.nextExplicitConsumerId );
			details::swap_relaxed( globalExplicitConsumerOffset, other.globalExplicitConsumerOffset );

			swap_implicit_producer_hashes( other );

			reown_producers();
			other.reown_producers();
		}

	public:
		inline bool enqueue( T && item )
		{
			if ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 ) return false;
			return inner_enqueue<CanAlloc>( std::move( item ) );
		}
		inline bool enqueue( T const & item )
		{
			if ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 ) return false;
			return inner_enqueue<CanAlloc>( item );
		}
		inline bool enqueue( producer_token_t const & token, T && item )
		{
			return inner_enqueue<CanAlloc>( token, std::move( item ) );
		}
		inline bool enqueue( producer_token_t const & token, T const & item )
		{
			return inner_enqueue<CanAlloc>( token, item );
		}

	public:
		template<typename It> bool enqueue_bulk( It itemFirst, size_t count )
		{
			if ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 ) return false;
			return inner_enqueue_bulk<CanAlloc>( itemFirst, count );
		}
		template<typename It> bool enqueue_bulk( producer_token_t const & token, It itemFirst, size_t count )
		{
			return inner_enqueue_bulk<CanAlloc>( token, itemFirst, count );
		}

	public:
		inline bool try_enqueue( T && item )
		{
			if ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 ) return false;
			return inner_enqueue<CannotAlloc>( std::move( item ) );
		}
		inline bool try_enqueue( T const & item )
		{
			if ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 ) return false;
			return inner_enqueue<CannotAlloc>( item );
		}
		inline bool try_enqueue( producer_token_t const & token, T && item )
		{
			return inner_enqueue<CannotAlloc>( token, std::move( item ) );
		}
		inline bool try_enqueue( producer_token_t const & token, T const & item )
		{
			return inner_enqueue<CannotAlloc>( token, item );
		}

	public:
		template<typename It> bool try_enqueue_bulk( It itemFirst, size_t count )
		{
			if ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 ) return false;
			return inner_enqueue_bulk<CannotAlloc>( itemFirst, count );
		}
		template<typename It> bool try_enqueue_bulk( producer_token_t const & token, It itemFirst, size_t count )
		{
			return inner_enqueue_bulk<CannotAlloc>( token, itemFirst, count );
		}

	public:
		template<typename U> bool try_dequeue( U & item )
		{
			size_t nonEmptyCount = 0;
			producer_base * best = nullptr;
			size_t bestSize = 0;
			for ( auto ptr = producerListTail.load( std::memory_order_acquire ); nonEmptyCount < 3 && ptr != nullptr; ptr = ptr->next_prod() )
			{
				auto size = ptr->size_approx();
				if ( size > 0 )
				{
					if ( size > bestSize )
					{
						bestSize = size;
						best = ptr;
					}
					++nonEmptyCount;
				}
			}

			if ( nonEmptyCount > 0 )
			{
				if ( best->dequeue( item ) )
				{
					return true;
				}
				for ( auto ptr = producerListTail.load( std::memory_order_acquire ); ptr != nullptr; ptr = ptr->next_prod() )
				{
					if ( ptr != best && ptr->dequeue( item ) )
					{
						return true;
					}
				}
			}
			return false;
		}
		template<typename U> bool try_dequeue( consumer_token_t & token, U & item )
		{
			if ( token.desiredProducer == nullptr || token.lastKnownGlobalOffset != globalExplicitConsumerOffset.load( std::memory_order_relaxed ) )
			{
				if ( !update_current_producer_after_rotation( token ) )
				{
					return false;
				}
			}

			if ( static_cast<producer_base *>( token.currentProducer )->dequeue( item ) )
			{
				if ( ++token.itemsConsumedFromCurrent == EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE )
				{
					globalExplicitConsumerOffset.fetch_add( 1, std::memory_order_relaxed );
				}
				return true;
			}

			auto tail = producerListTail.load( std::memory_order_acquire );
			auto ptr = static_cast<producer_base *>( token.currentProducer )->next_prod();
			if ( ptr == nullptr )
			{
				ptr = tail;
			}
			while ( ptr != static_cast<producer_base *>( token.currentProducer ) )
			{
				if ( ptr->dequeue( item ) )
				{
					token.currentProducer = ptr;
					token.itemsConsumedFromCurrent = 1;
					return true;
				}
				ptr = ptr->next_prod();
				if ( ptr == nullptr )
				{
					ptr = tail;
				}
			}
			return false;
		}

	public:
		template<typename It> size_t try_dequeue_bulk( It itemFirst, size_t max )
		{
			size_t count = 0;
			for ( auto ptr = producerListTail.load( std::memory_order_acquire ); ptr != nullptr; ptr = ptr->next_prod() )
			{
				count += ptr->dequeue_bulk( itemFirst, max - count );
				if ( count == max )
				{
					break;
				}
			}
			return count;
		}
		template<typename It> size_t try_dequeue_bulk( consumer_token_t & token, It itemFirst, size_t max )
		{
			if ( token.desiredProducer == nullptr || token.lastKnownGlobalOffset != globalExplicitConsumerOffset.load( std::memory_order_relaxed ) )
			{
				if ( !update_current_producer_after_rotation( token ) )
				{
					return 0;
				}
			}

			size_t count = static_cast<producer_base *>( token.currentProducer )->dequeue_bulk( itemFirst, max );
			if ( count == max )
			{
				if ( ( token.itemsConsumedFromCurrent += static_cast<std::uint32_t>( max ) ) >= EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE )
				{
					globalExplicitConsumerOffset.fetch_add( 1, std::memory_order_relaxed );
				}
				return max;
			}
			token.itemsConsumedFromCurrent += static_cast<std::uint32_t>( count );
			max -= count;

			auto tail = producerListTail.load( std::memory_order_acquire );
			auto ptr = static_cast<producer_base *>( token.currentProducer )->next_prod();
			if ( ptr == nullptr )
			{
				ptr = tail;
			}
			while ( ptr != static_cast<producer_base *>( token.currentProducer ) )
			{
				auto dequeued = ptr->dequeue_bulk( itemFirst, max );
				count += dequeued;
				if ( dequeued != 0 )
				{
					token.currentProducer = ptr;
					token.itemsConsumedFromCurrent = static_cast<std::uint32_t>( dequeued );
				}
				if ( dequeued == max )
				{
					break;
				}
				max -= dequeued;
				ptr = ptr->next_prod();
				if ( ptr == nullptr )
				{
					ptr = tail;
				}
			}
			return count;
		}

	public:
		template<typename U> bool try_dequeue_non_interleaved( U & item )
		{
			for ( auto ptr = producerListTail.load( std::memory_order_acquire ); ptr != nullptr; ptr = ptr->next_prod() )
			{
				if ( ptr->dequeue( item ) )
				{
					return true;
				}
			}
			return false;
		}
		template<typename U> bool try_dequeue_from_producer( producer_token_t const & producer, U & item )
		{
			return static_cast<explicit_producer *>( producer.producer )->dequeue( item );
		}
		template<typename It> size_t try_dequeue_bulk_from_producer( producer_token_t const & producer, It itemFirst, size_t max )
		{
			return static_cast<explicit_producer *>( producer.producer )->dequeue_bulk( itemFirst, max );
		}

	public:
		size_t size_approx() const
		{
			size_t size = 0;
			for ( auto ptr = producerListTail.load( std::memory_order_acquire ); ptr != nullptr; ptr = ptr->next_prod() )
			{
				size += ptr->size_approx();
			}
			return size;
		}

	private:
		friend struct producer_token;
		friend struct consumer_token;
		struct explicit_producer;
		friend struct explicit_producer;
		struct implicit_producer;
		friend struct implicit_producer;

	private:
		enum AllocationMode
		{
			CanAlloc, CannotAlloc
		};

	private:
		template<AllocationMode canAlloc, typename U> inline bool inner_enqueue( producer_token_t const & token, U && element )
		{
			return static_cast<explicit_producer *>( token.producer )->concurrent_queue::explicit_producer::template enqueue<canAlloc>( std::forward<U>( element ) );
		}

		template<AllocationMode canAlloc, typename U> inline bool inner_enqueue( U && element )
		{
			auto producer = get_or_add_implicit_producer();
			return producer == nullptr ? false : producer->concurrent_queue::implicit_producer::template enqueue<canAlloc>( std::forward<U>( element ) );
		}

		template<AllocationMode canAlloc, typename It> inline bool inner_enqueue_bulk( producer_token_t const & token, It itemFirst, size_t count )
		{
			return static_cast<explicit_producer *>( token.producer )->concurrent_queue::explicit_producer::template enqueue_bulk<canAlloc>( itemFirst, count );
		}

		template<AllocationMode canAlloc, typename It> inline bool inner_enqueue_bulk( It itemFirst, size_t count )
		{
			auto producer = get_or_add_implicit_producer();
			return producer == nullptr ? false : producer->concurrent_queue::implicit_producer::template enqueue_bulk<canAlloc>( itemFirst, count );
		}

		inline bool update_current_producer_after_rotation( consumer_token_t & token )
		{
			auto tail = producerListTail.load( std::memory_order_acquire );
			if ( token.desiredProducer == nullptr && tail == nullptr )
			{
				return false;
			}
			auto prodCount = producerCount.load( std::memory_order_relaxed );
			auto globalOffset = globalExplicitConsumerOffset.load( std::memory_order_relaxed );
			if ( token.desiredProducer == nullptr )
			{
				std::uint32_t offset = prodCount - 1 - ( token.initialOffset % prodCount );
				token.desiredProducer = tail;
				for ( std::uint32_t i = 0; i != offset; ++i )
				{
					token.desiredProducer = static_cast<producer_base *>( token.desiredProducer )->next_prod();
					if ( token.desiredProducer == nullptr )
					{
						token.desiredProducer = tail;
					}
				}
			}

			std::uint32_t delta = globalOffset - token.lastKnownGlobalOffset;
			if ( delta >= prodCount )
			{
				delta = delta % prodCount;
			}
			for ( std::uint32_t i = 0; i != delta; ++i )
			{
				token.desiredProducer = static_cast<producer_base *>( token.desiredProducer )->next_prod();
				if ( token.desiredProducer == nullptr )
				{
					token.desiredProducer = tail;
				}
			}

			token.lastKnownGlobalOffset = globalOffset;
			token.currentProducer = token.desiredProducer;
			token.itemsConsumedFromCurrent = 0;
			return true;
		}

	private:
		template <typename N> struct free_list_node
		{
			free_list_node() : freeListRefs( 0 ), freeListNext( nullptr )
			{
			}

			std::atomic<std::uint32_t> freeListRefs;
			std::atomic<N *> freeListNext;
		};

		template<typename N> struct free_list
		{
			free_list() : freeListHead( nullptr )
			{
			}
			free_list( free_list && other ) : freeListHead( other.freeListHead.load( std::memory_order_relaxed ) )
			{
				other.freeListHead.store( nullptr, std::memory_order_relaxed );
			}
			void swap( free_list & other )
			{
				details::swap_relaxed( freeListHead, other.freeListHead );
			}

			free_list( free_list const & ) = delete;
			free_list & operator=( free_list const & ) = delete;

			inline void add( N * node )
			{
				if ( node->freeListRefs.fetch_add( SHOULD_BE_ON_FREELIST, std::memory_order_acq_rel ) == 0 )
				{
					add_knowing_refcount_is_zero( node );
				}
			}

			inline N * try_get()
			{
				auto head = freeListHead.load( std::memory_order_acquire );
				while ( head != nullptr )
				{
					auto prevHead = head;
					auto refs = head->freeListRefs.load( std::memory_order_relaxed );
					if ( ( refs & REFS_MASK ) == 0 || !head->freeListRefs.compare_exchange_strong( refs, refs + 1, std::memory_order_acquire, std::memory_order_relaxed ) )
					{
						head = freeListHead.load( std::memory_order_acquire );
						continue;
					}

					auto next = head->freeListNext.load( std::memory_order_relaxed );
					if ( freeListHead.compare_exchange_strong( head, next, std::memory_order_acquire, std::memory_order_relaxed ) )
					{
						assert( ( head->freeListRefs.load( std::memory_order_relaxed ) & SHOULD_BE_ON_FREELIST ) == 0 );

						head->freeListRefs.fetch_sub( 2, std::memory_order_release );
						return head;
					}

					refs = prevHead->freeListRefs.fetch_sub( 1, std::memory_order_acq_rel );
					if ( refs == SHOULD_BE_ON_FREELIST + 1 )
					{
						add_knowing_refcount_is_zero( prevHead );
					}
				}

				return nullptr;
			}

			N * head_unsafe() const
			{
				return freeListHead.load( std::memory_order_relaxed );
			}

		private:
			inline void add_knowing_refcount_is_zero( N * node )
			{
				auto head = freeListHead.load( std::memory_order_relaxed );
				while ( true )
				{
					node->freeListNext.store( head, std::memory_order_relaxed );
					node->freeListRefs.store( 1, std::memory_order_release );
					if ( !freeListHead.compare_exchange_strong( head, node, std::memory_order_release, std::memory_order_relaxed ) )
					{
						if ( node->freeListRefs.fetch_add( SHOULD_BE_ON_FREELIST - 1, std::memory_order_release ) == 1 )
						{
							continue;
						}
					}
					return;
				}
			}

		private:
			std::atomic<N *> freeListHead;

			static constexpr std::uint32_t REFS_MASK = 0x7FFFFFFF;
			static constexpr std::uint32_t SHOULD_BE_ON_FREELIST = 0x80000000;
		};

		enum inner_queue_context
		{
			implicit_context = 0, explicit_context = 1
		};

		struct free_list_block
		{
			free_list_block()
				: next( nullptr ), elementsCompletelyDequeued( 0 ), freeListRefs( 0 ), freeListNext( nullptr ), shouldBeOnFreeList( false ), dynamicallyAllocated( true )
			{
			}

			template<inner_queue_context context> inline bool is_empty() const
			{
				if ( context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD )
				{
					for ( size_t i = 0; i < BLOCK_SIZE; ++i )
					{
						if ( !emptyFlags[i].load( std::memory_order_relaxed ) )
						{
							return false;
						}
					}

					std::atomic_thread_fence( std::memory_order_acquire );
					return true;
				}
				else
				{
					if ( elementsCompletelyDequeued.load( std::memory_order_relaxed ) == BLOCK_SIZE )
					{
						std::atomic_thread_fence( std::memory_order_acquire );
						return true;
					}
					assert( elementsCompletelyDequeued.load( std::memory_order_relaxed ) <= BLOCK_SIZE );
					return false;
				}
			}

			template<inner_queue_context context> inline bool set_empty( index_t i )
			{
				if ( context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD )
				{
					assert( !emptyFlags[BLOCK_SIZE - 1 - static_cast<size_t>( i & static_cast<index_t>( BLOCK_SIZE - 1 ) )].load( std::memory_order_relaxed ) );
					emptyFlags[BLOCK_SIZE - 1 - static_cast<size_t>( i & static_cast<index_t>( BLOCK_SIZE - 1 ) )].store( true, std::memory_order_release );
					return false;
				}
				else
				{
					auto prevVal = elementsCompletelyDequeued.fetch_add( 1, std::memory_order_release );
					assert( prevVal < BLOCK_SIZE );
					return prevVal == BLOCK_SIZE - 1;
				}
			}

			template<inner_queue_context context> inline bool set_many_empty( index_t i, size_t count )
			{
				if ( context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD )
				{
					std::atomic_thread_fence( std::memory_order_release );
					i = BLOCK_SIZE - 1 - static_cast<size_t>( i & static_cast<index_t>( BLOCK_SIZE - 1 ) ) - count + 1;
					for ( size_t j = 0; j != count; ++j )
					{
						assert( !emptyFlags[i + j].load( std::memory_order_relaxed ) );
						emptyFlags[i + j].store( true, std::memory_order_relaxed );
					}
					return false;
				}
				else
				{
					auto prevVal = elementsCompletelyDequeued.fetch_add( count, std::memory_order_release );
					assert( prevVal + count <= BLOCK_SIZE );
					return prevVal + count == BLOCK_SIZE;
				}
			}

			template<inner_queue_context context> inline void set_all_empty()
			{
				if ( context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD )
				{
					for ( size_t i = 0; i != BLOCK_SIZE; ++i )
					{
						emptyFlags[i].store( true, std::memory_order_relaxed );
					}
				}
				else
				{
					elementsCompletelyDequeued.store( BLOCK_SIZE, std::memory_order_relaxed );
				}
			}

			template<inner_queue_context context> inline void reset_empty()
			{
				if ( context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD )
				{
					for ( size_t i = 0; i != BLOCK_SIZE; ++i )
					{
						emptyFlags[i].store( false, std::memory_order_relaxed );
					}
				}
				else
				{
					elementsCompletelyDequeued.store( 0, std::memory_order_relaxed );
				}
			}

			inline T * operator[]( index_t idx ) noexcept
			{
				return static_cast<T *>( static_cast<void *>( elements ) ) + static_cast<size_t>( idx & static_cast<index_t>( BLOCK_SIZE - 1 ) );
			}
			inline T const * operator[]( index_t idx ) const noexcept
			{
				return static_cast<T const *>( static_cast<void const *>( elements ) ) + static_cast<size_t>( idx & static_cast<index_t>( BLOCK_SIZE - 1 ) );
			}

		private:
			static_assert( std::alignment_of<T>::value <= std::alignment_of<std::max_align_t>::value, "The queue does not support super-aligned types at this time" );

		private:
			char elements[sizeof( T ) * BLOCK_SIZE];

		public:
			free_list_block * next;
			std::atomic<size_t> elementsCompletelyDequeued;
			std::atomic<bool> emptyFlags[BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD ? BLOCK_SIZE : 1];
		public:
			std::atomic<std::uint32_t> freeListRefs;
			std::atomic<free_list_block *> freeListNext;
			std::atomic<bool> shouldBeOnFreeList;
			bool dynamicallyAllocated;
		};

		struct producer_base : public details::concurrent_queue_producer_typeless_base
		{
			producer_base( concurrent_queue * parent_, bool isExplicit_ ) :
				tailIndex( 0 ),
				headIndex( 0 ),
				dequeueOptimisticCount( 0 ),
				dequeueOvercommit( 0 ),
				tailBlock( nullptr ),
				isExplicit( isExplicit_ ),
				parent( parent_ )
			{
			}

			virtual ~producer_base()
			{
			};

			template<typename U> inline bool dequeue( U & element )
			{
				if ( isExplicit )
				{
					return static_cast<explicit_producer *>( this )->dequeue( element );
				}
				else
				{
					return static_cast<implicit_producer *>( this )->dequeue( element );
				}
			}

			template<typename It> inline size_t dequeue_bulk( It & itemFirst, size_t max )
			{
				if ( isExplicit )
				{
					return static_cast<explicit_producer *>( this )->dequeue_bulk( itemFirst, max );
				}
				else
				{
					return static_cast<implicit_producer *>( this )->dequeue_bulk( itemFirst, max );
				}
			}

			inline producer_base * next_prod() const
			{
				return static_cast<producer_base *>( next );
			}

			inline size_t size_approx() const
			{
				auto tail = tailIndex.load( std::memory_order_relaxed );
				auto head = headIndex.load( std::memory_order_relaxed );
				return details::circular_less_than( head, tail ) ? static_cast<size_t>( tail - head ) : 0;
			}

			inline index_t get_tail() const
			{
				return tailIndex.load( std::memory_order_relaxed );
			}

		protected:
			std::atomic<index_t> tailIndex;
			std::atomic<index_t> headIndex;

			std::atomic<index_t> dequeueOptimisticCount;
			std::atomic<index_t> dequeueOvercommit;

			free_list_block * tailBlock;

		public:
			bool isExplicit;
			concurrent_queue * parent;
		};

		struct explicit_producer : public producer_base
		{
			explicit explicit_producer( concurrent_queue * parent ) :
				producer_base( parent, true ),
				blockIndex( nullptr ),
				pr_blockIndexSlotsUsed( 0 ),
				pr_blockIndexSize( EXPLICIT_INITIAL_INDEX_SIZE >> 1 ),
				pr_blockIndexFront( 0 ),
				pr_blockIndexEntries( nullptr ),
				pr_blockIndexRaw( nullptr )
			{
				size_t poolBasedIndexSize = details::ceil_to_pow_2( parent->initialBlockPoolSize ) >> 1;
				if ( poolBasedIndexSize > pr_blockIndexSize )
				{
					pr_blockIndexSize = poolBasedIndexSize;
				}

				new_block_index( 0 );
			}

			~explicit_producer()
			{
				if ( this->tailBlock != nullptr )
				{
					free_list_block * halfDequeuedBlock = nullptr;
					if ( ( this->headIndex.load( std::memory_order_relaxed ) & static_cast<index_t>( BLOCK_SIZE - 1 ) ) != 0 )
					{
						size_t i = ( pr_blockIndexFront - pr_blockIndexSlotsUsed ) & ( pr_blockIndexSize - 1 );
						while ( details::circular_less_than<index_t>( pr_blockIndexEntries[i].base + BLOCK_SIZE, this->headIndex.load( std::memory_order_relaxed ) ) )
						{
							i = ( i + 1 ) & ( pr_blockIndexSize - 1 );
						}
						assert( details::circular_less_than<index_t>( pr_blockIndexEntries[i].base, this->headIndex.load( std::memory_order_relaxed ) ) );
						halfDequeuedBlock = pr_blockIndexEntries[i].block;
					}

					auto block = this->tailBlock;
					do
					{
						block = block->next;
						if ( block->concurrent_queue::free_list_block::template is_empty<explicit_context>() )
						{
							continue;
						}

						size_t i = 0;
						if ( block == halfDequeuedBlock )
						{
							i = static_cast<size_t>( this->headIndex.load( std::memory_order_relaxed ) & static_cast<index_t>( BLOCK_SIZE - 1 ) );
						}

						auto lastValidIndex = ( this->tailIndex.load( std::memory_order_relaxed ) & static_cast<index_t>( BLOCK_SIZE - 1 ) ) == 0 ? BLOCK_SIZE : static_cast<size_t>( this->tailIndex.load( std::memory_order_relaxed ) & static_cast<index_t>( BLOCK_SIZE - 1 ) );
						while ( i != BLOCK_SIZE && ( block != this->tailBlock || i != lastValidIndex ) )
						{
							( *block )[i++]->~T();
						}
					} while ( block != this->tailBlock );
				}

				if ( this->tailBlock != nullptr )
				{
					auto block = this->tailBlock;
					do
					{
						auto nextBlock = block->next;
						if ( block->dynamicallyAllocated )
						{
							destroy( block );
						}
						else
						{
							this->parent->add_block_to_free_list( block );
						}
						block = nextBlock;
					} while ( block != this->tailBlock );
				}

				auto header = static_cast<block_index_header *>( pr_blockIndexRaw );
				while ( header != nullptr )
				{
					auto prev = static_cast<block_index_header *>( header->prev );
					header->~block_index_header();
					(Traits::free)( header );
					header = prev;
				}
			}

			template<AllocationMode allocMode, typename U> inline bool enqueue( U && element )
			{
				index_t currentTailIndex = this->tailIndex.load( std::memory_order_relaxed );
				index_t newTailIndex = 1 + currentTailIndex;
				if ( ( currentTailIndex & static_cast<index_t>( BLOCK_SIZE - 1 ) ) == 0 )
				{
					auto startBlock = this->tailBlock;
					auto originalBlockIndexSlotsUsed = pr_blockIndexSlotsUsed;
					if ( this->tailBlock != nullptr && this->tailBlock->next->concurrent_queue::free_list_block::template is_empty<explicit_context>() )
					{
						this->tailBlock = this->tailBlock->next;
						this->tailBlock->concurrent_queue::free_list_block::template reset_empty<explicit_context>();
					}
					else
					{
						auto head = this->headIndex.load( std::memory_order_relaxed );
						assert( !details::circular_less_than<index_t>( currentTailIndex, head ) );
						if ( !details::circular_less_than<index_t>( head, currentTailIndex + BLOCK_SIZE )
							 || ( MAX_SUBQUEUE_SIZE != std::numeric_limits<size_t>::max() && ( MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head ) ) )
						{
							return false;
						}

						if ( pr_blockIndexRaw == nullptr || pr_blockIndexSlotsUsed == pr_blockIndexSize )
						{
							if ( allocMode == CannotAlloc || !new_block_index( pr_blockIndexSlotsUsed ) )
							{
								return false;
							}
						}

						auto newBlock = this->parent->concurrent_queue::template requisition_block<allocMode>();
						if ( newBlock == nullptr )
						{
							return false;
						}

						newBlock->concurrent_queue::free_list_block::template reset_empty<explicit_context>();
						if ( this->tailBlock == nullptr )
						{
							newBlock->next = newBlock;
						}
						else
						{
							newBlock->next = this->tailBlock->next;
							this->tailBlock->next = newBlock;
						}
						this->tailBlock = newBlock;
						++pr_blockIndexSlotsUsed;
					}

					if ( !noexcept( new ( ( T * )nullptr ) T( std::forward<U>( element ) ) ) )
					{
						try
						{
							new ( ( *this->tailBlock )[currentTailIndex] ) T( std::forward<U>( element ) );
						}
						catch ( ... )
						{
							pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
							this->tailBlock = startBlock == nullptr ? this->tailBlock : startBlock;
							throw;
						}
					}
					else
					{
						(void)startBlock;
						(void)originalBlockIndexSlotsUsed;
					}

					auto & entry = blockIndex.load( std::memory_order_relaxed )->entries[pr_blockIndexFront];
					entry.base = currentTailIndex;
					entry.block = this->tailBlock;
					blockIndex.load( std::memory_order_relaxed )->front.store( pr_blockIndexFront, std::memory_order_release );
					pr_blockIndexFront = ( pr_blockIndexFront + 1 ) & ( pr_blockIndexSize - 1 );

					if ( !noexcept( new ( ( T * )nullptr ) T( std::forward<U>( element ) ) ) )
					{
						this->tailIndex.store( newTailIndex, std::memory_order_release );
						return true;
					}
				}

				new ( ( *this->tailBlock )[currentTailIndex] ) T( std::forward<U>( element ) );

				this->tailIndex.store( newTailIndex, std::memory_order_release );
				return true;
			}

			template<typename U> bool dequeue( U & element )
			{
				auto tail = this->tailIndex.load( std::memory_order_relaxed );
				auto overcommit = this->dequeueOvercommit.load( std::memory_order_relaxed );
				if ( details::circular_less_than<index_t>( this->dequeueOptimisticCount.load( std::memory_order_relaxed ) - overcommit, tail ) )
				{
					std::atomic_thread_fence( std::memory_order_acquire );

					auto myDequeueCount = this->dequeueOptimisticCount.fetch_add( 1, std::memory_order_relaxed );

					tail = this->tailIndex.load( std::memory_order_acquire );
					if ( details::circular_less_than<index_t>( myDequeueCount - overcommit, tail ) )
					{
						auto index = this->headIndex.fetch_add( 1, std::memory_order_acq_rel );
						auto localBlockIndex = blockIndex.load( std::memory_order_acquire );
						auto localBlockIndexHead = localBlockIndex->front.load( std::memory_order_acquire );
						auto headBase = localBlockIndex->entries[localBlockIndexHead].base;
						auto blockBaseIndex = index & ~static_cast<index_t>( BLOCK_SIZE - 1 );
						auto offset = static_cast<size_t>( static_cast<typename std::make_signed<index_t>::type>( blockBaseIndex - headBase ) / BLOCK_SIZE );
						auto block = localBlockIndex->entries[( localBlockIndexHead + offset ) & ( localBlockIndex->size - 1 )].block;

						auto & el = *( ( *block )[index] );
						if ( !noexcept( element = std::move( el ) ) )
						{
							struct _guard
							{
								free_list_block * block;
								index_t index;

								~_guard()
								{
									( *block )[index]->~T();
									block->concurrent_queue::free_list_block::template set_empty<explicit_context>( index );
								}
							} guard = { block, index };

							element = std::move( el );
						}
						else
						{
							element = std::move( el );
							el.~T();
							block->concurrent_queue::free_list_block::template set_empty<explicit_context>( index );
						}

						return true;
					}
					else
					{
						this->dequeueOvercommit.fetch_add( 1, std::memory_order_release );
					}
				}

				return false;
			}

			template<AllocationMode allocMode, typename It> bool enqueue_bulk( It itemFirst, size_t count )
			{
				index_t startTailIndex = this->tailIndex.load( std::memory_order_relaxed );
				auto startBlock = this->tailBlock;
				auto originalBlockIndexFront = pr_blockIndexFront;
				auto originalBlockIndexSlotsUsed = pr_blockIndexSlotsUsed;

				free_list_block * firstAllocatedBlock = nullptr;

				size_t blockBaseDiff = ( ( startTailIndex + count - 1 ) & ~static_cast<index_t>( BLOCK_SIZE - 1 ) ) - ( ( startTailIndex - 1 ) & ~static_cast<index_t>( BLOCK_SIZE - 1 ) );
				index_t currentTailIndex = ( startTailIndex - 1 ) & ~static_cast<index_t>( BLOCK_SIZE - 1 );
				if ( blockBaseDiff > 0 )
				{
					while ( blockBaseDiff > 0 && this->tailBlock != nullptr && this->tailBlock->next != firstAllocatedBlock && this->tailBlock->next->concurrent_queue::free_list_block::template is_empty<explicit_context>() )
					{
						blockBaseDiff -= static_cast<index_t>( BLOCK_SIZE );
						currentTailIndex += static_cast<index_t>( BLOCK_SIZE );

						this->tailBlock = this->tailBlock->next;
						firstAllocatedBlock = firstAllocatedBlock == nullptr ? this->tailBlock : firstAllocatedBlock;

						auto & entry = blockIndex.load( std::memory_order_relaxed )->entries[pr_blockIndexFront];
						entry.base = currentTailIndex;
						entry.block = this->tailBlock;
						pr_blockIndexFront = ( pr_blockIndexFront + 1 ) & ( pr_blockIndexSize - 1 );
					}

					while ( blockBaseDiff > 0 )
					{
						blockBaseDiff -= static_cast<index_t>( BLOCK_SIZE );
						currentTailIndex += static_cast<index_t>( BLOCK_SIZE );

						auto head = this->headIndex.load( std::memory_order_relaxed );
						assert( !details::circular_less_than<index_t>( currentTailIndex, head ) );
						bool full = !details::circular_less_than<index_t>( head, currentTailIndex + BLOCK_SIZE ) || ( MAX_SUBQUEUE_SIZE != std::numeric_limits<size_t>::max() && ( MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head ) );
						if ( pr_blockIndexRaw == nullptr || pr_blockIndexSlotsUsed == pr_blockIndexSize || full )
						{
							if ( allocMode == CannotAlloc || full || !new_block_index( originalBlockIndexSlotsUsed ) )
							{
								pr_blockIndexFront = originalBlockIndexFront;
								pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
								this->tailBlock = startBlock == nullptr ? firstAllocatedBlock : startBlock;
								return false;
							}

							originalBlockIndexFront = originalBlockIndexSlotsUsed;
						}

						auto newBlock = this->parent->concurrent_queue::template requisition_block<allocMode>();
						if ( newBlock == nullptr )
						{
							pr_blockIndexFront = originalBlockIndexFront;
							pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
							this->tailBlock = startBlock == nullptr ? firstAllocatedBlock : startBlock;
							return false;
						}

						newBlock->concurrent_queue::free_list_block::template set_all_empty<explicit_context>();
						if ( this->tailBlock == nullptr )
						{
							newBlock->next = newBlock;
						}
						else
						{
							newBlock->next = this->tailBlock->next;
							this->tailBlock->next = newBlock;
						}
						this->tailBlock = newBlock;
						firstAllocatedBlock = firstAllocatedBlock == nullptr ? this->tailBlock : firstAllocatedBlock;

						++pr_blockIndexSlotsUsed;

						auto & entry = blockIndex.load( std::memory_order_relaxed )->entries[pr_blockIndexFront];
						entry.base = currentTailIndex;
						entry.block = this->tailBlock;
						pr_blockIndexFront = ( pr_blockIndexFront + 1 ) & ( pr_blockIndexSize - 1 );
					}

					auto block = firstAllocatedBlock;
					while ( true )
					{
						block->concurrent_queue::free_list_block::template reset_empty<explicit_context>();
						if ( block == this->tailBlock )
						{
							break;
						}
						block = block->next;
					}

					if ( noexcept( new ( ( T * )nullptr ) T( details::deref_noexcept( itemFirst ) ) ) )
					{
						blockIndex.load( std::memory_order_relaxed )->front.store( ( pr_blockIndexFront - 1 ) & ( pr_blockIndexSize - 1 ), std::memory_order_release );
					}
				}

				index_t newTailIndex = startTailIndex + static_cast<index_t>( count );
				currentTailIndex = startTailIndex;
				auto endBlock = this->tailBlock;
				this->tailBlock = startBlock;
				assert( ( startTailIndex & static_cast<index_t>( BLOCK_SIZE - 1 ) ) != 0 || firstAllocatedBlock != nullptr || count == 0 );
				if ( ( startTailIndex & static_cast<index_t>( BLOCK_SIZE - 1 ) ) == 0 && firstAllocatedBlock != nullptr )
				{
					this->tailBlock = firstAllocatedBlock;
				}
				while ( true )
				{
					auto stopIndex = ( currentTailIndex & ~static_cast<index_t>( BLOCK_SIZE - 1 ) ) + static_cast<index_t>( BLOCK_SIZE );
					if ( details::circular_less_than<index_t>( newTailIndex, stopIndex ) )
					{
						stopIndex = newTailIndex;
					}
					if ( noexcept( new ( ( T * )nullptr ) T( details::deref_noexcept( itemFirst ) ) ) )
					{
						while ( currentTailIndex != stopIndex )
						{
							new ( ( *this->tailBlock )[currentTailIndex++] ) T( *itemFirst++ );
						}
					}
					else
					{
						try
						{
							while ( currentTailIndex != stopIndex )
							{
								new ( ( *this->tailBlock )[currentTailIndex] ) T( details::nomove_if<(bool)!noexcept( new ( ( T * )nullptr ) T( details::deref_noexcept( itemFirst ) ) )>::eval( *itemFirst ) );
								++currentTailIndex;
								++itemFirst;
							}
						}
						catch ( ... )
						{
							auto constructedStopIndex = currentTailIndex;
							auto lastBlockEnqueued = this->tailBlock;

							pr_blockIndexFront = originalBlockIndexFront;
							pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
							this->tailBlock = startBlock == nullptr ? firstAllocatedBlock : startBlock;

							if ( !std::is_trivially_destructible<T>::value )
							{
								auto block = startBlock;
								if ( ( startTailIndex & static_cast<index_t>( BLOCK_SIZE - 1 ) ) == 0 )
								{
									block = firstAllocatedBlock;
								}
								currentTailIndex = startTailIndex;
								while ( true )
								{
									stopIndex = ( currentTailIndex & ~static_cast<index_t>( BLOCK_SIZE - 1 ) ) + static_cast<index_t>( BLOCK_SIZE );
									if ( details::circular_less_than<index_t>( constructedStopIndex, stopIndex ) )
									{
										stopIndex = constructedStopIndex;
									}
									while ( currentTailIndex != stopIndex )
									{
										( *block )[currentTailIndex++]->~T();
									}
									if ( block == lastBlockEnqueued )
									{
										break;
									}
									block = block->next;
								}
							}
							throw;
						}
					}

					if ( this->tailBlock == endBlock )
					{
						assert( currentTailIndex == newTailIndex );
						break;
					}
					this->tailBlock = this->tailBlock->next;
				}

				if ( !noexcept( new ( ( T * )nullptr ) T( details::deref_noexcept( itemFirst ) ) ) && firstAllocatedBlock != nullptr )
				{
					blockIndex.load( std::memory_order_relaxed )->front.store( ( pr_blockIndexFront - 1 ) & ( pr_blockIndexSize - 1 ), std::memory_order_release );
				}

				this->tailIndex.store( newTailIndex, std::memory_order_release );
				return true;
			}

			template<typename It> size_t dequeue_bulk( It & itemFirst, size_t max )
			{
				auto tail = this->tailIndex.load( std::memory_order_relaxed );
				auto overcommit = this->dequeueOvercommit.load( std::memory_order_relaxed );
				auto desiredCount = static_cast<size_t>( tail - ( this->dequeueOptimisticCount.load( std::memory_order_relaxed ) - overcommit ) );
				if ( details::circular_less_than<size_t>( 0, desiredCount ) )
				{
					desiredCount = desiredCount < max ? desiredCount : max;
					std::atomic_thread_fence( std::memory_order_acquire );

					auto myDequeueCount = this->dequeueOptimisticCount.fetch_add( desiredCount, std::memory_order_relaxed );;

					tail = this->tailIndex.load( std::memory_order_acquire );
					auto actualCount = static_cast<size_t>( tail - ( myDequeueCount - overcommit ) );
					if ( details::circular_less_than<size_t>( 0, actualCount ) )
					{
						actualCount = desiredCount < actualCount ? desiredCount : actualCount;
						if ( actualCount < desiredCount )
						{
							this->dequeueOvercommit.fetch_add( desiredCount - actualCount, std::memory_order_release );
						}

						auto firstIndex = this->headIndex.fetch_add( actualCount, std::memory_order_acq_rel );
						auto localBlockIndex = blockIndex.load( std::memory_order_acquire );
						auto localBlockIndexHead = localBlockIndex->front.load( std::memory_order_acquire );

						auto headBase = localBlockIndex->entries[localBlockIndexHead].base;
						auto firstBlockBaseIndex = firstIndex & ~static_cast<index_t>( BLOCK_SIZE - 1 );
						auto offset = static_cast<size_t>( static_cast<typename std::make_signed<index_t>::type>( firstBlockBaseIndex - headBase ) / BLOCK_SIZE );
						auto indexIndex = ( localBlockIndexHead + offset ) & ( localBlockIndex->size - 1 );

						auto index = firstIndex;
						do
						{
							auto firstIndexInBlock = index;
							auto endIndex = ( index & ~static_cast<index_t>( BLOCK_SIZE - 1 ) ) + static_cast<index_t>( BLOCK_SIZE );
							endIndex = details::circular_less_than<index_t>( firstIndex + static_cast<index_t>( actualCount ), endIndex ) ? firstIndex + static_cast<index_t>( actualCount ) : endIndex;
							auto block = localBlockIndex->entries[indexIndex].block;
							if ( noexcept( details::deref_noexcept( itemFirst ) = std::move( ( *( *block )[index] ) ) ) )
							{
								while ( index != endIndex )
								{
									auto & el = *( ( *block )[index] );
									*itemFirst++ = std::move( el );
									el.~T();
									++index;
								}
							}
							else
							{
								try
								{
									while ( index != endIndex )
									{
										auto & el = *( ( *block )[index] );
										*itemFirst = std::move( el );
										++itemFirst;
										el.~T();
										++index;
									}
								}
								catch ( ... )
								{
									do
									{
										block = localBlockIndex->entries[indexIndex].block;
										while ( index != endIndex )
										{
											( *block )[index++]->~T();
										}
										block->concurrent_queue::free_list_block::template set_many_empty<explicit_context>( firstIndexInBlock, static_cast<size_t>( endIndex - firstIndexInBlock ) );
										indexIndex = ( indexIndex + 1 ) & ( localBlockIndex->size - 1 );

										firstIndexInBlock = index;
										endIndex = ( index & ~static_cast<index_t>( BLOCK_SIZE - 1 ) ) + static_cast<index_t>( BLOCK_SIZE );
										endIndex = details::circular_less_than<index_t>( firstIndex + static_cast<index_t>( actualCount ), endIndex ) ? firstIndex + static_cast<index_t>( actualCount ) : endIndex;
									} while ( index != firstIndex + actualCount );

									throw;
								}
							}
							block->concurrent_queue::free_list_block::template set_many_empty<explicit_context>( firstIndexInBlock, static_cast<size_t>( endIndex - firstIndexInBlock ) );
							indexIndex = ( indexIndex + 1 ) & ( localBlockIndex->size - 1 );
						} while ( index != firstIndex + actualCount );

						return actualCount;
					}
					else
					{
						this->dequeueOvercommit.fetch_add( desiredCount, std::memory_order_release );
					}
				}

				return 0;
			}

		private:
			struct block_index_entry
			{
				index_t base;
				free_list_block * block;
			};

			struct block_index_header
			{
				size_t size;
				std::atomic<size_t> front;
				block_index_entry * entries;
				void * prev;
			};

		private:
			bool new_block_index( size_t numberOfFilledSlotsToExpose )
			{
				auto prevBlockSizeMask = pr_blockIndexSize - 1;

				pr_blockIndexSize <<= 1;
				auto newRawPtr = static_cast<char *>( (Traits::malloc)( sizeof( block_index_header ) + std::alignment_of<block_index_entry>::value - 1 + sizeof( block_index_entry ) * pr_blockIndexSize ) );
				if ( newRawPtr == nullptr )
				{
					pr_blockIndexSize >>= 1;
					return false;
				}

				auto newBlockIndexEntries = reinterpret_cast<block_index_entry *>( details::align_for<block_index_entry>( newRawPtr + sizeof( block_index_header ) ) );

				size_t j = 0;
				if ( pr_blockIndexSlotsUsed != 0 )
				{
					auto i = ( pr_blockIndexFront - pr_blockIndexSlotsUsed ) & prevBlockSizeMask;
					do
					{
						newBlockIndexEntries[j++] = pr_blockIndexEntries[i];
						i = ( i + 1 ) & prevBlockSizeMask;
					} while ( i != pr_blockIndexFront );
				}

				auto header = new ( newRawPtr ) block_index_header;
				header->size = pr_blockIndexSize;
				header->front.store( numberOfFilledSlotsToExpose - 1, std::memory_order_relaxed );
				header->entries = newBlockIndexEntries;
				header->prev = pr_blockIndexRaw;

				pr_blockIndexFront = j;
				pr_blockIndexEntries = newBlockIndexEntries;
				pr_blockIndexRaw = newRawPtr;
				blockIndex.store( header, std::memory_order_release );

				return true;
			}

		private:
			std::atomic<block_index_header *> blockIndex;

			size_t pr_blockIndexSlotsUsed;
			size_t pr_blockIndexSize;
			size_t pr_blockIndexFront;
			block_index_entry * pr_blockIndexEntries;
			void * pr_blockIndexRaw;
		};

		struct implicit_producer : public producer_base
		{
			implicit_producer( concurrent_queue * parent ) :
				producer_base( parent, false ),
				nextBlockIndexCapacity( IMPLICIT_INITIAL_INDEX_SIZE ),
				blockIndex( nullptr )
			{
				new_block_index();
			}

			~implicit_producer()
			{
				auto tail = this->tailIndex.load( std::memory_order_relaxed );
				auto index = this->headIndex.load( std::memory_order_relaxed );
				free_list_block * block = nullptr;
				assert( index == tail || details::circular_less_than( index, tail ) );
				bool forceFreeLastBlock = index != tail;
				while ( index != tail )
				{
					if ( ( index & static_cast<index_t>( BLOCK_SIZE - 1 ) ) == 0 || block == nullptr )
					{
						if ( block != nullptr )
						{
							this->parent->add_block_to_free_list( block );
						}

						block = get_block_index_entry_for_index( index )->value.load( std::memory_order_relaxed );
					}

					( ( *block )[index] )->~T();
					++index;
				}

				if ( this->tailBlock != nullptr && ( forceFreeLastBlock || ( tail & static_cast<index_t>( BLOCK_SIZE - 1 ) ) != 0 ) )
				{
					this->parent->add_block_to_free_list( this->tailBlock );
				}

				auto localBlockIndex = blockIndex.load( std::memory_order_relaxed );
				if ( localBlockIndex != nullptr )
				{
					for ( size_t i = 0; i != localBlockIndex->capacity; ++i )
					{
						localBlockIndex->index[i]->~block_index_entry();
					}
					do
					{
						auto prev = localBlockIndex->prev;
						localBlockIndex->~block_index_header();
						(Traits::free)( localBlockIndex );
						localBlockIndex = prev;
					} while ( localBlockIndex != nullptr );
				}
			}

			template<AllocationMode allocMode, typename U> inline bool enqueue( U && element )
			{
				index_t currentTailIndex = this->tailIndex.load( std::memory_order_relaxed );
				index_t newTailIndex = 1 + currentTailIndex;
				if ( ( currentTailIndex & static_cast<index_t>( BLOCK_SIZE - 1 ) ) == 0 )
				{
					auto head = this->headIndex.load( std::memory_order_relaxed );
					assert( !details::circular_less_than<index_t>( currentTailIndex, head ) );
					if ( !details::circular_less_than<index_t>( head, currentTailIndex + BLOCK_SIZE ) || ( MAX_SUBQUEUE_SIZE != std::numeric_limits<size_t>::max() && ( MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head ) ) )
					{
						return false;
					}

					block_index_entry * idxEntry;
					if ( !insert_block_index_entry<allocMode>( idxEntry, currentTailIndex ) )
					{
						return false;
					}

					auto newBlock = this->parent->concurrent_queue::template requisition_block<allocMode>();
					if ( newBlock == nullptr )
					{
						rewind_block_index_tail();
						idxEntry->value.store( nullptr, std::memory_order_relaxed );
						return false;
					}
					newBlock->concurrent_queue::free_list_block::template reset_empty<implicit_context>();

					if ( !noexcept( new ( ( T * )nullptr ) T( std::forward<U>( element ) ) ) )
					{
						try
						{
							new ( ( *newBlock )[currentTailIndex] ) T( std::forward<U>( element ) );
						}
						catch ( ... )
						{
							rewind_block_index_tail();
							idxEntry->value.store( nullptr, std::memory_order_relaxed );
							this->parent->add_block_to_free_list( newBlock );
							throw;
						}
					}

					idxEntry->value.store( newBlock, std::memory_order_relaxed );

					this->tailBlock = newBlock;

					if ( !noexcept( new ( ( T * )nullptr ) T( std::forward<U>( element ) ) ) )
					{
						this->tailIndex.store( newTailIndex, std::memory_order_release );
						return true;
					}
				}

				new ( ( *this->tailBlock )[currentTailIndex] ) T( std::forward<U>( element ) );

				this->tailIndex.store( newTailIndex, std::memory_order_release );
				return true;
			}

			template<typename U> bool dequeue( U & element )
			{
				index_t tail = this->tailIndex.load( std::memory_order_relaxed );
				index_t overcommit = this->dequeueOvercommit.load( std::memory_order_relaxed );
				if ( details::circular_less_than<index_t>( this->dequeueOptimisticCount.load( std::memory_order_relaxed ) - overcommit, tail ) )
				{
					std::atomic_thread_fence( std::memory_order_acquire );

					index_t myDequeueCount = this->dequeueOptimisticCount.fetch_add( 1, std::memory_order_relaxed );
					tail = this->tailIndex.load( std::memory_order_acquire );
					if ( details::circular_less_than<index_t>( myDequeueCount - overcommit, tail ) )
					{
						index_t index = this->headIndex.fetch_add( 1, std::memory_order_acq_rel );

						auto entry = get_block_index_entry_for_index( index );

						auto block = entry->value.load( std::memory_order_relaxed );
						auto & el = *( ( *block )[index] );

						if ( !noexcept( element = std::move( el ) ) )
						{
							struct _guard
							{
								free_list_block * block;
								index_t index;
								block_index_entry * entry;
								concurrent_queue * parent;

								~_guard()
								{
									( *block )[index]->~T();
									if ( block->concurrent_queue::free_list_block::template set_empty<implicit_context>( index ) )
									{
										entry->value.store( nullptr, std::memory_order_relaxed );
										parent->add_block_to_free_list( block );
									}
								}
							} guard = { block, index, entry, this->parent };

							element = std::move( el );
						}
						else
						{
							element = std::move( el ); 							el.~T();
							if ( block->concurrent_queue::free_list_block::template set_empty<implicit_context>( index ) )
							{
								{
									entry->value.store( nullptr, std::memory_order_relaxed );
								}
								this->parent->add_block_to_free_list( block );
							}
						}

						return true;
					}
					else
					{
						this->dequeueOvercommit.fetch_add( 1, std::memory_order_release );
					}
				}

				return false;
			}

			template<AllocationMode allocMode, typename It> bool enqueue_bulk( It itemFirst, size_t count )
			{
				index_t startTailIndex = this->tailIndex.load( std::memory_order_relaxed );
				auto startBlock = this->tailBlock;
				free_list_block * firstAllocatedBlock = nullptr;
				auto endBlock = this->tailBlock;

				size_t blockBaseDiff = ( ( startTailIndex + count - 1 ) & ~static_cast<index_t>( BLOCK_SIZE - 1 ) ) - ( ( startTailIndex - 1 ) & ~static_cast<index_t>( BLOCK_SIZE - 1 ) );
				index_t currentTailIndex = ( startTailIndex - 1 ) & ~static_cast<index_t>( BLOCK_SIZE - 1 );
				if ( blockBaseDiff > 0 )
				{
					do
					{
						blockBaseDiff -= static_cast<index_t>( BLOCK_SIZE );
						currentTailIndex += static_cast<index_t>( BLOCK_SIZE );

						block_index_entry * idxEntry = nullptr;
						free_list_block * newBlock;
						bool indexInserted = false;
						auto head = this->headIndex.load( std::memory_order_relaxed );
						assert( !details::circular_less_than<index_t>( currentTailIndex, head ) );
						bool full = !details::circular_less_than<index_t>( head, currentTailIndex + BLOCK_SIZE ) || ( MAX_SUBQUEUE_SIZE != std::numeric_limits<size_t>::max() && ( MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head ) );
						if ( full || !( indexInserted = insert_block_index_entry<allocMode>( idxEntry, currentTailIndex ) ) || ( newBlock = this->parent->concurrent_queue::template requisition_block<allocMode>() ) == nullptr )
						{
							if ( indexInserted )
							{
								rewind_block_index_tail();
								idxEntry->value.store( nullptr, std::memory_order_relaxed );
							}
							currentTailIndex = ( startTailIndex - 1 ) & ~static_cast<index_t>( BLOCK_SIZE - 1 );
							for ( auto block = firstAllocatedBlock; block != nullptr; block = block->next )
							{
								currentTailIndex += static_cast<index_t>( BLOCK_SIZE );
								idxEntry = get_block_index_entry_for_index( currentTailIndex );
								idxEntry->value.store( nullptr, std::memory_order_relaxed );
								rewind_block_index_tail();
							}
							this->parent->add_blocks_to_free_list( firstAllocatedBlock );
							this->tailBlock = startBlock;

							return false;
						}

						newBlock->concurrent_queue::free_list_block::template reset_empty<implicit_context>();
						newBlock->next = nullptr;

						idxEntry->value.store( newBlock, std::memory_order_relaxed );

						if ( ( startTailIndex & static_cast<index_t>( BLOCK_SIZE - 1 ) ) != 0 || firstAllocatedBlock != nullptr )
						{
							assert( this->tailBlock != nullptr );
							this->tailBlock->next = newBlock;
						}
						this->tailBlock = newBlock;
						endBlock = newBlock;
						firstAllocatedBlock = firstAllocatedBlock == nullptr ? newBlock : firstAllocatedBlock;
					} while ( blockBaseDiff > 0 );
				}

				index_t newTailIndex = startTailIndex + static_cast<index_t>( count );
				currentTailIndex = startTailIndex;
				this->tailBlock = startBlock;
				assert( ( startTailIndex & static_cast<index_t>( BLOCK_SIZE - 1 ) ) != 0 || firstAllocatedBlock != nullptr || count == 0 );
				if ( ( startTailIndex & static_cast<index_t>( BLOCK_SIZE - 1 ) ) == 0 && firstAllocatedBlock != nullptr )
				{
					this->tailBlock = firstAllocatedBlock;
				}
				while ( true )
				{
					auto stopIndex = ( currentTailIndex & ~static_cast<index_t>( BLOCK_SIZE - 1 ) ) + static_cast<index_t>( BLOCK_SIZE );
					if ( details::circular_less_than<index_t>( newTailIndex, stopIndex ) )
					{
						stopIndex = newTailIndex;
					}
					if ( noexcept( new ( ( T * )nullptr ) T( details::deref_noexcept( itemFirst ) ) ) )
					{
						while ( currentTailIndex != stopIndex )
						{
							new ( ( *this->tailBlock )[currentTailIndex++] ) T( *itemFirst++ );
						}
					}
					else
					{
						try
						{
							while ( currentTailIndex != stopIndex )
							{
								new ( ( *this->tailBlock )[currentTailIndex] ) T( details::nomove_if<(bool)!noexcept( new ( ( T * )nullptr ) T( details::deref_noexcept( itemFirst ) ) )>::eval( *itemFirst ) );
								++currentTailIndex;
								++itemFirst;
							}
						}
						catch ( ... )
						{
							auto constructedStopIndex = currentTailIndex;
							auto lastBlockEnqueued = this->tailBlock;

							if ( !std::is_trivially_destructible<T>::value )
							{
								auto block = startBlock;
								if ( ( startTailIndex & static_cast<index_t>( BLOCK_SIZE - 1 ) ) == 0 )
								{
									block = firstAllocatedBlock;
								}
								currentTailIndex = startTailIndex;
								while ( true )
								{
									stopIndex = ( currentTailIndex & ~static_cast<index_t>( BLOCK_SIZE - 1 ) ) + static_cast<index_t>( BLOCK_SIZE );
									if ( details::circular_less_than<index_t>( constructedStopIndex, stopIndex ) )
									{
										stopIndex = constructedStopIndex;
									}
									while ( currentTailIndex != stopIndex )
									{
										( *block )[currentTailIndex++]->~T();
									}
									if ( block == lastBlockEnqueued )
									{
										break;
									}
									block = block->next;
								}
							}

							currentTailIndex = ( startTailIndex - 1 ) & ~static_cast<index_t>( BLOCK_SIZE - 1 );
							for ( auto block = firstAllocatedBlock; block != nullptr; block = block->next )
							{
								currentTailIndex += static_cast<index_t>( BLOCK_SIZE );
								auto idxEntry = get_block_index_entry_for_index( currentTailIndex );
								idxEntry->value.store( nullptr, std::memory_order_relaxed );
								rewind_block_index_tail();
							}
							this->parent->add_blocks_to_free_list( firstAllocatedBlock );
							this->tailBlock = startBlock;
							throw;
						}
					}

					if ( this->tailBlock == endBlock )
					{
						assert( currentTailIndex == newTailIndex );
						break;
					}
					this->tailBlock = this->tailBlock->next;
				}
				this->tailIndex.store( newTailIndex, std::memory_order_release );
				return true;
			}

			template<typename It> size_t dequeue_bulk( It & itemFirst, size_t max )
			{
				auto tail = this->tailIndex.load( std::memory_order_relaxed );
				auto overcommit = this->dequeueOvercommit.load( std::memory_order_relaxed );
				auto desiredCount = static_cast<size_t>( tail - ( this->dequeueOptimisticCount.load( std::memory_order_relaxed ) - overcommit ) );
				if ( details::circular_less_than<size_t>( 0, desiredCount ) )
				{
					desiredCount = desiredCount < max ? desiredCount : max;
					std::atomic_thread_fence( std::memory_order_acquire );

					auto myDequeueCount = this->dequeueOptimisticCount.fetch_add( desiredCount, std::memory_order_relaxed );

					tail = this->tailIndex.load( std::memory_order_acquire );
					auto actualCount = static_cast<size_t>( tail - ( myDequeueCount - overcommit ) );
					if ( details::circular_less_than<size_t>( 0, actualCount ) )
					{
						actualCount = desiredCount < actualCount ? desiredCount : actualCount;
						if ( actualCount < desiredCount )
						{
							this->dequeueOvercommit.fetch_add( desiredCount - actualCount, std::memory_order_release );
						}

						auto firstIndex = this->headIndex.fetch_add( actualCount, std::memory_order_acq_rel );

						auto index = firstIndex;
						block_index_header * localBlockIndex;
						auto indexIndex = get_block_index_index_for_index( index, localBlockIndex );
						do
						{
							auto blockStartIndex = index;
							auto endIndex = ( index & ~static_cast<index_t>( BLOCK_SIZE - 1 ) ) + static_cast<index_t>( BLOCK_SIZE );
							endIndex = details::circular_less_than<index_t>( firstIndex + static_cast<index_t>( actualCount ), endIndex ) ? firstIndex + static_cast<index_t>( actualCount ) : endIndex;

							auto entry = localBlockIndex->index[indexIndex];
							auto block = entry->value.load( std::memory_order_relaxed );
							if ( noexcept( details::deref_noexcept( itemFirst ) = std::move( ( *( *block )[index] ) ) ) )
							{
								while ( index != endIndex )
								{
									auto & el = *( ( *block )[index] );
									*itemFirst++ = std::move( el );
									el.~T();
									++index;
								}
							}
							else
							{
								try
								{
									while ( index != endIndex )
									{
										auto & el = *( ( *block )[index] );
										*itemFirst = std::move( el );
										++itemFirst;
										el.~T();
										++index;
									}
								}
								catch ( ... )
								{
									do
									{
										entry = localBlockIndex->index[indexIndex];
										block = entry->value.load( std::memory_order_relaxed );
										while ( index != endIndex )
										{
											( *block )[index++]->~T();
										}

										if ( block->concurrent_queue::free_list_block::template set_many_empty<implicit_context>( blockStartIndex, static_cast<size_t>( endIndex - blockStartIndex ) ) )
										{
											entry->value.store( nullptr, std::memory_order_relaxed );
											this->parent->add_block_to_free_list( block );
										}
										indexIndex = ( indexIndex + 1 ) & ( localBlockIndex->capacity - 1 );

										blockStartIndex = index;
										endIndex = ( index & ~static_cast<index_t>( BLOCK_SIZE - 1 ) ) + static_cast<index_t>( BLOCK_SIZE );
										endIndex = details::circular_less_than<index_t>( firstIndex + static_cast<index_t>( actualCount ), endIndex ) ? firstIndex + static_cast<index_t>( actualCount ) : endIndex;
									} while ( index != firstIndex + actualCount );

									throw;
								}
							}
							if ( block->concurrent_queue::free_list_block::template set_many_empty<implicit_context>( blockStartIndex, static_cast<size_t>( endIndex - blockStartIndex ) ) )
							{
								{
									entry->value.store( nullptr, std::memory_order_relaxed );
								}
								this->parent->add_block_to_free_list( block );
							}
							indexIndex = ( indexIndex + 1 ) & ( localBlockIndex->capacity - 1 );
						} while ( index != firstIndex + actualCount );

						return actualCount;
					}
					else
					{
						this->dequeueOvercommit.fetch_add( desiredCount, std::memory_order_release );
					}
				}

				return 0;
			}

		private:
			static constexpr index_t INVALID_BLOCK_BASE = 1;

			struct block_index_entry
			{
				std::atomic<index_t> key;
				std::atomic<free_list_block *> value;
			};

			struct block_index_header
			{
				size_t capacity;
				std::atomic<size_t> tail;
				block_index_entry * entries;
				block_index_entry ** index;
				block_index_header * prev;
			};

			template<AllocationMode allocMode> inline bool insert_block_index_entry( block_index_entry *& idxEntry, index_t blockStartIndex )
			{
				auto localBlockIndex = blockIndex.load( std::memory_order_relaxed );						if ( localBlockIndex == nullptr )
				{
					return false;
				}
				auto newTail = ( localBlockIndex->tail.load( std::memory_order_relaxed ) + 1 ) & ( localBlockIndex->capacity - 1 );
				idxEntry = localBlockIndex->index[newTail];
				if ( idxEntry->key.load( std::memory_order_relaxed ) == INVALID_BLOCK_BASE ||
					 idxEntry->value.load( std::memory_order_relaxed ) == nullptr )
				{

					idxEntry->key.store( blockStartIndex, std::memory_order_relaxed );
					localBlockIndex->tail.store( newTail, std::memory_order_release );
					return true;
				}

				if ( allocMode == CannotAlloc || !new_block_index() )
				{
					return false;
				}
				localBlockIndex = blockIndex.load( std::memory_order_relaxed );
				newTail = ( localBlockIndex->tail.load( std::memory_order_relaxed ) + 1 ) & ( localBlockIndex->capacity - 1 );
				idxEntry = localBlockIndex->index[newTail];
				assert( idxEntry->key.load( std::memory_order_relaxed ) == INVALID_BLOCK_BASE );
				idxEntry->key.store( blockStartIndex, std::memory_order_relaxed );
				localBlockIndex->tail.store( newTail, std::memory_order_release );
				return true;
			}

			inline void rewind_block_index_tail()
			{
				auto localBlockIndex = blockIndex.load( std::memory_order_relaxed );
				localBlockIndex->tail.store( ( localBlockIndex->tail.load( std::memory_order_relaxed ) - 1 ) & ( localBlockIndex->capacity - 1 ), std::memory_order_relaxed );
			}

			inline block_index_entry * get_block_index_entry_for_index( index_t index ) const
			{
				block_index_header * localBlockIndex;
				auto idx = get_block_index_index_for_index( index, localBlockIndex );
				return localBlockIndex->index[idx];
			}

			inline size_t get_block_index_index_for_index( index_t index, block_index_header *& localBlockIndex ) const
			{
				index &= ~static_cast<index_t>( BLOCK_SIZE - 1 );
				localBlockIndex = blockIndex.load( std::memory_order_acquire );
				auto tail = localBlockIndex->tail.load( std::memory_order_acquire );
				auto tailBase = localBlockIndex->index[tail]->key.load( std::memory_order_relaxed );
				assert( tailBase != INVALID_BLOCK_BASE );
				auto offset = static_cast<size_t>( static_cast<typename std::make_signed<index_t>::type>( index - tailBase ) / BLOCK_SIZE );
				size_t idx = ( tail + offset ) & ( localBlockIndex->capacity - 1 );
				assert( localBlockIndex->index[idx]->key.load( std::memory_order_relaxed ) == index && localBlockIndex->index[idx]->value.load( std::memory_order_relaxed ) != nullptr );
				return idx;
			}

			bool new_block_index()
			{
				auto prev = blockIndex.load( std::memory_order_relaxed );
				size_t prevCapacity = prev == nullptr ? 0 : prev->capacity;
				auto entryCount = prev == nullptr ? nextBlockIndexCapacity : prevCapacity;
				auto raw = static_cast<char *>( (Traits::malloc)(
					sizeof( block_index_header ) +
					std::alignment_of<block_index_entry>::value - 1 + sizeof( block_index_entry ) * entryCount +
					std::alignment_of<block_index_entry *>::value - 1 + sizeof( block_index_entry * ) * nextBlockIndexCapacity ) );
				if ( raw == nullptr )
				{
					return false;
				}

				auto header = new ( raw ) block_index_header;
				auto entries = reinterpret_cast<block_index_entry *>( details::align_for<block_index_entry>( raw + sizeof( block_index_header ) ) );
				auto index = reinterpret_cast<block_index_entry **>( details::align_for<block_index_entry *>( reinterpret_cast<char *>( entries ) + sizeof( block_index_entry ) * entryCount ) );
				if ( prev != nullptr )
				{
					auto prevTail = prev->tail.load( std::memory_order_relaxed );
					auto prevPos = prevTail;
					size_t i = 0;
					do
					{
						prevPos = ( prevPos + 1 ) & ( prev->capacity - 1 );
						index[i++] = prev->index[prevPos];
					} while ( prevPos != prevTail );
					assert( i == prevCapacity );
				}
				for ( size_t i = 0; i != entryCount; ++i )
				{
					new ( entries + i ) block_index_entry;
					entries[i].key.store( INVALID_BLOCK_BASE, std::memory_order_relaxed );
					index[prevCapacity + i] = entries + i;
				}
				header->prev = prev;
				header->entries = entries;
				header->index = index;
				header->capacity = nextBlockIndexCapacity;
				header->tail.store( ( prevCapacity - 1 ) & ( nextBlockIndexCapacity - 1 ), std::memory_order_relaxed );

				blockIndex.store( header, std::memory_order_release );

				nextBlockIndexCapacity <<= 1;

				return true;
			}

		private:
			size_t nextBlockIndexCapacity;
			std::atomic<block_index_header *> blockIndex;
		};

	private:
		void populate_initial_block_list( size_t blockCount )
		{
			initialBlockPoolSize = blockCount;
			if ( initialBlockPoolSize == 0 )
			{
				initialBlockPool = nullptr;
				return;
			}

			initialBlockPool = create_array<free_list_block>( blockCount );
			if ( initialBlockPool == nullptr )
			{
				initialBlockPoolSize = 0;
			}
			for ( size_t i = 0; i < initialBlockPoolSize; ++i )
			{
				initialBlockPool[i].dynamicallyAllocated = false;
			}
		}

		inline free_list_block * try_get_block_from_initial_pool()
		{
			if ( initialBlockPoolIndex.load( std::memory_order_relaxed ) >= initialBlockPoolSize )
			{
				return nullptr;
			}

			auto index = initialBlockPoolIndex.fetch_add( 1, std::memory_order_relaxed );

			return index < initialBlockPoolSize ? ( initialBlockPool + index ) : nullptr;
		}

		inline void add_block_to_free_list( free_list_block * block )
		{
			freeList.add( block );
		}

		inline void add_blocks_to_free_list( free_list_block * block )
		{
			while ( block != nullptr )
			{
				auto next = block->next;
				add_block_to_free_list( block );
				block = next;
			}
		}

		inline free_list_block * try_get_block_from_free_list()
		{
			return freeList.try_get();
		}

		template<AllocationMode canAlloc> free_list_block * requisition_block()
		{
			auto block = try_get_block_from_initial_pool();
			if ( block != nullptr )
			{
				return block;
			}

			block = try_get_block_from_free_list();
			if ( block != nullptr )
			{
				return block;
			}

			if ( canAlloc == CanAlloc )
			{
				return create<free_list_block>();
			}

			return nullptr;
		}

		producer_base * recycle_or_create_producer( bool isExplicit )
		{
			bool recycled;
			return recycle_or_create_producer( isExplicit, recycled );
		}

		producer_base * recycle_or_create_producer( bool isExplicit, bool & recycled )
		{
			for ( auto ptr = producerListTail.load( std::memory_order_acquire ); ptr != nullptr; ptr = ptr->next_prod() )
			{
				if ( ptr->inactive.load( std::memory_order_relaxed ) && ptr->isExplicit == isExplicit )
				{
					bool expected = true;
					if ( ptr->inactive.compare_exchange_strong( expected, /* desired */ false, std::memory_order_acquire, std::memory_order_relaxed ) )
					{
						recycled = true;
						return ptr;
					}
				}
			}

			recycled = false;
			return add_producer( isExplicit ? static_cast<producer_base *>( create<explicit_producer>( this ) ) : create<implicit_producer>( this ) );
		}

		producer_base * add_producer( producer_base * producer )
		{
			if ( producer == nullptr )
			{
				return nullptr;
			}

			producerCount.fetch_add( 1, std::memory_order_relaxed );

			auto prevTail = producerListTail.load( std::memory_order_relaxed );
			do
			{
				producer->next = prevTail;
			} while ( !producerListTail.compare_exchange_weak( prevTail, producer, std::memory_order_release, std::memory_order_relaxed ) );

			return producer;
		}

		void reown_producers()
		{
			for ( auto ptr = producerListTail.load( std::memory_order_relaxed ); ptr != nullptr; ptr = ptr->next_prod() )
			{
				ptr->parent = this;
			}
		}

	private:
		struct implicit_producer_kvp
		{
			std::atomic<std::thread::id> key;
			implicit_producer * value;
			implicit_producer_kvp() : value( nullptr )
			{
			}

			implicit_producer_kvp( implicit_producer_kvp && other ) noexcept
			{
				key.store( other.key.load( std::memory_order_relaxed ), std::memory_order_relaxed );
				value = other.value;
			}

			inline implicit_producer_kvp & operator=( implicit_producer_kvp && other ) noexcept
			{
				swap( other );
				return *this;
			}

			inline void swap( implicit_producer_kvp & other ) noexcept
			{
				if ( this != &other )
				{
					details::swap_relaxed( key, other.key );
					std::swap( value, other.value );
				}
			}
		};

		template<typename XT, typename XTraits> friend void x::swap( typename concurrent_queue<XT, XTraits>::implicit_producer_kvp &, typename concurrent_queue<XT, XTraits>::implicit_producer_kvp & ) noexcept;

		struct implicit_producer_hash
		{
			size_t capacity;
			implicit_producer_kvp * entries;
			implicit_producer_hash * prev;
		};

	private:
		inline void populate_initial_implicit_producer_hash()
		{
			if ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 ) return;

			implicitProducerHashCount.store( 0, std::memory_order_relaxed );
			auto hash = &initialImplicitProducerHash;
			hash->capacity = INITIAL_IMPLICIT_PRODUCER_HASH_SIZE;
			hash->entries = &initialImplicitProducerHashEntries[0];
			for ( size_t i = 0; i != INITIAL_IMPLICIT_PRODUCER_HASH_SIZE; ++i )
			{
				initialImplicitProducerHashEntries[i].key.store( std::thread::id(), std::memory_order_relaxed );
			}
			hash->prev = nullptr;
			implicitProducerHash.store( hash, std::memory_order_relaxed );
		}

		void swap_implicit_producer_hashes( concurrent_queue & other )
		{
			if ( INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 ) return;

			initialImplicitProducerHashEntries.swap( other.initialImplicitProducerHashEntries );
			initialImplicitProducerHash.entries = &initialImplicitProducerHashEntries[0];
			other.initialImplicitProducerHash.entries = &other.initialImplicitProducerHashEntries[0];

			details::swap_relaxed( implicitProducerHashCount, other.implicitProducerHashCount );

			details::swap_relaxed( implicitProducerHash, other.implicitProducerHash );
			if ( implicitProducerHash.load( std::memory_order_relaxed ) == &other.initialImplicitProducerHash )
			{
				implicitProducerHash.store( &initialImplicitProducerHash, std::memory_order_relaxed );
			}
			else
			{
				implicit_producer_hash * hash;
				for ( hash = implicitProducerHash.load( std::memory_order_relaxed ); hash->prev != &other.initialImplicitProducerHash; hash = hash->prev )
				{
					continue;
				}
				hash->prev = &initialImplicitProducerHash;
			}
			if ( other.implicitProducerHash.load( std::memory_order_relaxed ) == &initialImplicitProducerHash )
			{
				other.implicitProducerHash.store( &other.initialImplicitProducerHash, std::memory_order_relaxed );
			}
			else
			{
				implicit_producer_hash * hash;
				for ( hash = other.implicitProducerHash.load( std::memory_order_relaxed ); hash->prev != &initialImplicitProducerHash; hash = hash->prev )
				{
					continue;
				}
				hash->prev = &other.initialImplicitProducerHash;
			}
		}

		implicit_producer * get_or_add_implicit_producer()
		{
			auto id = std::this_thread::get_id();
			auto hashedId = std::hash<std::thread::id>()( id );

			auto mainHash = implicitProducerHash.load( std::memory_order_acquire );
			for ( auto hash = mainHash; hash != nullptr; hash = hash->prev )
			{
				auto index = hashedId;
				while ( true )
				{
					index &= hash->capacity - 1;

					auto probedKey = hash->entries[index].key.load( std::memory_order_relaxed );
					if ( probedKey == id )
					{
						auto value = hash->entries[index].value;
						if ( hash != mainHash )
						{
							index = hashedId;
							while ( true )
							{
								index &= mainHash->capacity - 1;
								probedKey = mainHash->entries[index].key.load( std::memory_order_relaxed );
								auto empty = std::thread::id();

								if ( ( probedKey == empty && mainHash->entries[index].key.compare_exchange_strong( empty, id, std::memory_order_relaxed, std::memory_order_relaxed ) ) )
								{
									mainHash->entries[index].value = value;
									break;
								}
								++index;
							}
						}

						return value;
					}
					if ( probedKey == std::thread::id() )
					{
						break;
					}
					++index;
				}
			}

			auto newCount = 1 + implicitProducerHashCount.fetch_add( 1, std::memory_order_relaxed );
			while ( true )
			{
				if ( newCount >= ( mainHash->capacity >> 1 ) && !implicitProducerHashResizeInProgress.test_and_set( std::memory_order_acquire ) )
				{
					mainHash = implicitProducerHash.load( std::memory_order_acquire );
					if ( newCount >= ( mainHash->capacity >> 1 ) )
					{
						auto newCapacity = mainHash->capacity << 1;
						while ( newCount >= ( newCapacity >> 1 ) )
						{
							newCapacity <<= 1;
						}
						auto raw = static_cast<char *>( (Traits::malloc)( sizeof( implicit_producer_hash ) + std::alignment_of<implicit_producer_kvp>::value - 1 + sizeof( implicit_producer_kvp ) * newCapacity ) );
						if ( raw == nullptr )
						{
							implicitProducerHashCount.fetch_sub( 1, std::memory_order_relaxed );
							implicitProducerHashResizeInProgress.clear( std::memory_order_relaxed );
							return nullptr;
						}

						auto newHash = new ( raw ) implicit_producer_hash;
						newHash->capacity = newCapacity;
						newHash->entries = reinterpret_cast<implicit_producer_kvp *>( details::align_for<implicit_producer_kvp>( raw + sizeof( implicit_producer_hash ) ) );
						for ( size_t i = 0; i != newCapacity; ++i )
						{
							new ( newHash->entries + i ) implicit_producer_kvp;
							newHash->entries[i].key.store( std::thread::id(), std::memory_order_relaxed );
						}
						newHash->prev = mainHash;
						implicitProducerHash.store( newHash, std::memory_order_release );
						implicitProducerHashResizeInProgress.clear( std::memory_order_release );
						mainHash = newHash;
					}
					else
					{
						implicitProducerHashResizeInProgress.clear( std::memory_order_release );
					}
				}

				if ( newCount < ( mainHash->capacity >> 1 ) + ( mainHash->capacity >> 2 ) )
				{
					bool recycled;
					auto producer = static_cast<implicit_producer *>( recycle_or_create_producer( false, recycled ) );
					if ( producer == nullptr )
					{
						implicitProducerHashCount.fetch_sub( 1, std::memory_order_relaxed );
						return nullptr;
					}
					if ( recycled )
					{
						implicitProducerHashCount.fetch_sub( 1, std::memory_order_relaxed );
					}

					auto index = hashedId;
					while ( true )
					{
						index &= mainHash->capacity - 1;
						auto probedKey = mainHash->entries[index].key.load( std::memory_order_relaxed );

						auto empty = std::thread::id();

						if ( ( probedKey == empty && mainHash->entries[index].key.compare_exchange_strong( empty, id, std::memory_order_relaxed, std::memory_order_relaxed ) ) )
						{
							mainHash->entries[index].value = producer;
							break;
						}
						++index;
					}
					return producer;
				}

				mainHash = implicitProducerHash.load( std::memory_order_acquire );
			}
		}

	private:
		template<typename U> static inline U * create_array( size_t count )
		{
			assert( count > 0 );
			auto p = static_cast<U *>( (Traits::malloc)( sizeof( U ) * count ) );
			if ( p == nullptr )
			{
				return nullptr;
			}

			for ( size_t i = 0; i != count; ++i )
			{
				new ( p + i ) U();
			}
			return p;
		}

		template<typename U> static inline void destroy_array( U * p, size_t count )
		{
			if ( p != nullptr )
			{
				assert( count > 0 );
				for ( size_t i = count; i != 0; )
				{
					( p + --i )->~U();
				}
				(Traits::free)( p );
			}
		}

		template<typename U> static inline U * create()
		{
			auto p = (Traits::malloc)( sizeof( U ) );
			return p != nullptr ? new ( p ) U : nullptr;
		}

		template<typename U, typename A1> static inline U * create( A1 && a1 )
		{
			auto p = (Traits::malloc)( sizeof( U ) );
			return p != nullptr ? new ( p ) U( std::forward<A1>( a1 ) ) : nullptr;
		}

		template<typename U> static inline void destroy( U * p )
		{
			if ( p != nullptr )
			{
				p->~U();
			}
			(Traits::free)( p );
		}

	private:
		std::atomic<producer_base *> producerListTail;
		std::atomic<std::uint32_t> producerCount;

		std::atomic<size_t> initialBlockPoolIndex;
		free_list_block * initialBlockPool;
		size_t initialBlockPoolSize;
		free_list<free_list_block> freeList;

		std::atomic<implicit_producer_hash *> implicitProducerHash;
		std::atomic<size_t> implicitProducerHashCount;				implicit_producer_hash initialImplicitProducerHash;
		std::array<implicit_producer_kvp, INITIAL_IMPLICIT_PRODUCER_HASH_SIZE> initialImplicitProducerHashEntries;
		std::atomic_flag implicitProducerHashResizeInProgress;

		std::atomic<std::uint32_t> nextExplicitConsumerId;
		std::atomic<std::uint32_t> globalExplicitConsumerOffset;
	};

	template<typename T, typename Traits> producer_token::producer_token( concurrent_queue<T, Traits> & queue )
		: producer( queue.recycle_or_create_producer( true ) )
	{
		if ( producer != nullptr )
		{
			producer->token = this;
		}
	}

	template<typename T, typename Traits> consumer_token::consumer_token( concurrent_queue<T, Traits> & queue )
		: itemsConsumedFromCurrent( 0 ), currentProducer( nullptr ), desiredProducer( nullptr )
	{
		initialOffset = queue.nextExplicitConsumerId.fetch_add( 1, std::memory_order_release );
		lastKnownGlobalOffset = -1;
	}

	template<typename T, typename Traits> inline void swap( concurrent_queue<T, Traits> & a, concurrent_queue<T, Traits> & b ) noexcept
	{
		a.swap( b );
	}

	inline void swap( producer_token & a, producer_token & b ) noexcept
	{
		a.swap( b );
	}

	inline void swap( consumer_token & a, consumer_token & b ) noexcept
	{
		a.swap( b );
	}

	template<typename T, typename Traits> inline void swap( typename concurrent_queue<T, Traits>::implicit_producer_kvp & a, typename concurrent_queue<T, Traits>::implicit_producer_kvp & b ) noexcept
	{
		a.swap( b );
	}
}
