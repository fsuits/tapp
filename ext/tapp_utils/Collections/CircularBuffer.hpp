#pragma once
#include <algorithm>
#include <exception>
#include <iterator>
#include <mutex>
#include <vector>

namespace TAPP::Collections
{
	template <class T>
	/// <summary>This class offers a contiguous collection that can be appended and consumed without resizing the underlying structure.</summary>
	class CircularBuffer
	{
		public:
			class iterator;

		private:
			std::mutex		m_access_mutex_;
			std::vector<T>	m_elements_;
			size_t			m_count_;
			size_t			m_start_index_;

		public:
			/// <summary>Initializes the CircularBuffer with the defined capacity.</summary>
			/// <param name="capacity">The amount of elements this CircularBuffer will hold.</param>
			CircularBuffer(const size_t capacity) : m_elements_(capacity)
			{
			}

			/// <summary>Copy constructs the CircularBuffer.</summary>
			/// <param name="other">Another CircularBuffer of the same type.</param>
			CircularBuffer(const CircularBuffer& other) : m_elements_(other.m_elements_), m_count_(other.m_count_), m_start_index_(other.m_start_index_)
			{
			}

			/// <summary>Move constructs the CircularBuffer.</summary>
			/// <param name="other">Another CircularBuffer of the same type.</param>
			CircularBuffer(CircularBuffer&& other) noexcept : m_elements_(std::move(other.m_elements_)), m_count_(other.m_count_), m_start_index_(other.m_start_index_)
			{
				other.m_count_			= 0;
				other.m_start_index_	= 0;
			}

			/// <summary>Destructs the CircularBuffer, ensuring the mutex is no longer locked.</summary>
			~CircularBuffer(void)
			{
				// Ensures there are no operations being performed on this buffer.
				m_access_mutex_.lock();
				m_access_mutex_.unlock();
			}

			/// <summary>Copies the variables from another CircularBuffer.</summary>
			/// <param name="other">Another CircularBuffer of the same type.</param>
			CircularBuffer& operator= (const CircularBuffer& other)
			{
				m_elements_		= other.m_elements_;
				m_count_		= other.m_count_;
				m_start_index_	= other.m_start_index_;

				return *this;
			}

			/// <summary>Moves the variables from another CircularBuffer to this one.</summary>
			/// <param name="other">Another CircularBuffer of the same type.</param>
			CircularBuffer& operator= (CircularBuffer&& other) noexcept
			{
				m_elements_				= std::move(other.m_elements_);
				m_count_				= other.m_count_;
				m_start_index_			= other.m_start_index_;

				other.m_count_			= 0;
				other.m_start_index_	= 0;

				return *this;
			}

			/// <summary>Adds a single element to the CircularBuffer.</summary>
			/// <param name="value">The value to add to the CircularBuffer.</param>
			void Add(const T value)
			{
				CheckSize_(1);
				m_elements_[(m_start_index_ + m_count_) % m_elements_.size()] = value;
				++m_count_;
			}

			/// <summary>Adds the defined range of elements from the passed array.</summary>
			/// <param name="collection_start">The start of the collection holding the elements to copy.</param>
			/// <param name="count">The amount of elements to copy.</param>
			void Add(const T* collection_start, const size_t count)
			{
				// Checks if the amount of additional elements will exceed the capacity of this CircularBuffer.
				CheckSize_(count);

				// Adds the additional elements.
				m_access_mutex_.lock();

				// If the added elements won't surpass the vector end.
				if (m_start_index_ + m_count_ + count < m_elements_.size())
				{
					std::copy_n(collection_start, count, m_elements_.begin() + m_start_index_ + m_count_);
				}
				// If the added elements will require the start of the vector.
				else
				{
					// Calculates how many elements require insertion before and after the end.
					size_t pre_end_count	= m_elements_.size() - (m_start_index_ + m_count_);
					size_t post_end_count	= count - pre_end_count;

					std::copy_n(collection_start, pre_end_count, m_elements_.begin() + m_start_index_ + m_count_);
					std::copy_n(collection_start + pre_end_count, post_end_count, m_elements_.begin());
				}

				// Increases the counter to reflect the current amount of elements.
				m_count_ += count;
				m_access_mutex_.unlock();
			}

			/// <summary>Copies a part of a vector into the buffer.</summary>
			/// <param name="source">The vector to copy the elements from.</param>
			/// <param name="count">The amount of elements to copy.</param>
			/// <param name="offset">The index of the starting element for the copying.</param>
			void Add(const std::vector<T>& source, size_t count = 0, const size_t offset = 0)
			{
				// If no size has been defined, assume the entire vector has to be acquired.
				if (count == 0)
				{
					count = source.size();
				}

				// Adds the additional elements.
				Add(&(*(source.begin() + offset)), count);
			}

			/// <summary>Clears the CircularBuffer of all elements.</summary>
			void Clear(void)
			{
				m_access_mutex_.lock();
				m_count_ = 0;
				m_access_mutex_.unlock();
			}

			/// <summary>Consumes a defined range of elements.</summary>
			/// <param name="count">The amount of elements to remove from this buffer.</param>
			void Consume(const size_t count)
			{
				if (count >= m_count_)
				{
					Clear();
				}
				else
				{
					m_access_mutex_.lock();
					IncreaseIndex_(count);
					m_count_ -= count;
					m_access_mutex_.unlock();
				}
			}

			/// <summary>Manually increases the amount of claimed elements within the buffer.</summary>
			/// <param name="count">The amount of elements to claim.</param>
			void IncreaseCount(size_t count)
			{
				CheckSize_(count);
				m_count_ += count;
			}

			/// <summary>Returns the current amount of elements within the buffer.</summary>
			/// <returns>The current amount of elements within the buffer.</returns>
			size_t Size(void)
			{
				m_access_mutex_.lock();
				size_t current_count = m_count_;
				m_access_mutex_.unlock();
				return current_count;
			}

			/// <summary>Returns the maximum amount of elements this buffer can hold.</summary>
			/// <returns>The maximum amount of elements this buffer can hold.</returns>
			size_t MaximumSize(void)
			{
				return m_elements_.size();
			}
			
			/// <summary>Returns the amount of unclaimed elements before the internal vector loops back on itself.</summary>
			/// <returns>The amount of unclaimed elements at the back of the internal vector.</returns>
			size_t UnclaimedElementsAtBack(void)
			{
				if (m_start_index_ + m_count_ < m_elements_.size())
				{
					return m_elements_.size() - (m_start_index_ + m_count_);
				}
				return 0;
			}

			/// <summary>Returns an iterator pointing at the start of the buffer or the first element.</summary>
			/// <returns>An iterator pointing at the start of the buffer or the first element.</returns>
			iterator begin(void)
			{
				m_access_mutex_.lock();
				iterator it(this, m_start_index_, 0);
				m_access_mutex_.unlock();

				return it;
			}

			/// <summary>Returns an iterator that points to an imaginary element behind the current final element.</summary>
			/// <returns>An iterator that points to an imaginary element behind the current final element.</returns>
			iterator end(void)
			{
				if (m_count_ == 0)
				{
					return begin();
				}

				m_access_mutex_.lock();
				iterator it(this, (m_start_index_ + m_count_) % m_elements_.size(), m_count_);
				m_access_mutex_.unlock();

				return it;
			}

			/// <summary>Attempts to access an element based on index.</summary>
			/// <param name="element">The index of the element to access.</param>
			/// <returns>A reference to the requested element.</returns>
			T& operator[] (size_t element)
			{
				return m_elements_[(m_start_index_ + element) % m_elements_.size()];
			}

			/// <summary>Attempts to access an element based on index.</summary>
			/// <param name="element">The index of the element to access.</param>
			/// <returns>A constant reference to the requested element.</returns>
			const T& operator[] (size_t element) const
			{
				return m_elements_[(m_start_index_ + element) % m_elements_.size()];
			}

		private:
			/// <summary>Checks if the added range will exceed the capacity of this CircularBuffer.</summary>
			/// <param name="count">The amount of elements that are theoretically added.</param>
			void CheckSize_(const size_t count)
			{
				// If the count exceeds the current elements.
				if (m_count_ + count > m_elements_.size())
				{
					throw std::overflow_error("The amount of additional elements will exceed the capacity of the CircularBuffer.");
				}
			}

			/// <summary>Increases the starting index of the buffer, taking into account the end of the vector.</summary>
			/// <param name="count">The amount of elements the index should shift.</param>
			void IncreaseIndex_(const size_t count)
			{
				m_start_index_ = (m_start_index_ + count) % m_elements_.size();
			}

			/// <summary>Returns a recalculated index.</summary>
			/// <param name="current_index">The starting index.</param>
			/// <param name="count">The value to modify the index with.</param>
			/// <returns>The recalculated index.</returns>
			size_t CalculateIndex_(const size_t current_index, const int64_t count) const
			{
				return (current_index + count) % m_elements_.size();
			}

		public:
			// TODO: Ensure there's no undefined behaviour.
			/// <summary>An iterator that can transcend the CircularBuffer elements.</summary>
			class iterator // : public std::iterator<std::random_access_iterator_tag, T>
			{
				private:
					size_t						m_count_; // Required to differentiate between begin and end on a full buffer.
					size_t						m_element_index_;
					const CircularBuffer<T>*	m_source_;

				public:
					typedef T							value_type;
					typedef std::ptrdiff_t				difference_type;
					typedef T*							pointer;
					typedef T&							reference;
					typedef std::input_iterator_tag		iterator_category;

					/// <summary>Initializes the default iterator, lacking any concrete information.</summary>
					iterator(void) : m_source_(nullptr), m_element_index_(0)
					{
					}

					/// <summary>Initializes the iterator.</summary>
					/// <param name="source">The source CircularBuffer.</param>
					/// <param name="element_index">The element index directly aimed at an element in the CircularBuffer.m_elements_ object.</param>
					/// <param name="count">The total amount of elements when regarded from the start index.</param>
					iterator(const CircularBuffer<T>* source, size_t element_index, size_t count) : m_source_(source), m_element_index_(element_index), m_count_(count)
					{
						if (m_element_index_ >= m_source_->m_elements_.size() || m_count_ > m_source_->m_elements_.size())
						{
							throw std::runtime_error("Iterator values do not fit within source buffer constraints.");
						}
					}

					/// <summary>Swaps the iterator information from the passed iterator.</summary>
					/// <param name="other">Another iterator of the same type.</param>
					void swap(iterator& other)
					{
						// Holds the values of this iterator.
						size_t						this_count		= m_count_;
						size_t						this_index		= m_element_index_;
						const CircularBuffer<T>*	this_source		= m_source_;

						// Replaces the values of this iterator.
						m_count_				= other.m_count_;
						m_element_index_		= other.m_element_index_;
						m_source_				= other.m_source_;

						// Replaces the values of the other iterator.
						other.m_count_			= this_count;
						other.m_element_index_	= this_index;
						other.m_source_			= this_source;
					}

					/// <summary>Returns a reference to the element that the iterator represents.</summary>
					/// <returns>A reference to the element the iterator points towards.</returns>
					const T& operator* (void) const
					{
						return m_source_->m_elements_[m_element_index_];
					}

					/// <summary>Returns a reference to the element that the iterator represents.</summary>
					/// <returns>A reference to the element the iterator points towards.</returns>
					const T& operator-> (void) const
					{
						return m_source_->m_elements_[m_element_index_];
					}

					/// <summary>Returns the result of addition.</summary>
					/// <param name="increment">The amount to increment the current iterator by.</param>
					/// <returns>An iterator pointing at the current element_index + the increment.</returns>
					const iterator operator+ (int increment)
					{
						return iterator(m_source_, m_source_->CalculateIndex_(m_element_index_, increment), m_count_ + increment);
					}

					/// <summary>Returns the result of addition.</summary>
					/// <param name="other">Another iterator, which provides an increment value through its element_index.</param>
					/// <returns>An iterator pointing at the current element_index + the increment.</returns>
					ptrdiff_t operator+ (const iterator& other)
					{
						return m_count_ - other.m_count_;
					}

					/// <summary>Returns the result of subtraction.</summary>
					/// <param name="increment">The amount to subtract the current iterator by.</param>
					/// <returns>An iterator pointing at the current element_index - the increment.</returns>
					const iterator operator- (int decrement)
					{
						return iterator(m_source_, m_source_->CalculateIndex_(m_element_index_, -1 * decrement), m_count_ - decrement);
					}

					/// <summary>Returns the result of subtraction.</summary>
					/// <param name="other">Another iterator, which provides an subtract value through its element_index.</param>
					/// <returns>An iterator pointing at the current element_index - the increment.</returns>
					ptrdiff_t operator- (const iterator& other)
					{
						return m_count_ - other.m_count_;
					}

					/// <summary>Increments the current element_index of the iterator by the value passed.</summary>
					/// <param name="decrement">The amount to add to the current element_index.</param>
					void operator+= (int increment)
					{
						CheckConstraints_(increment);
						m_count_			+= increment;
						m_element_index_	= m_source_->CalculateIndex_(m_element_index_, increment);
					}

					/// <summary>Increments the current element_index of the iterator by the element_index of the passed iterator.</summary>
					/// <param name="other">Another iterator of the same type.</param>
					void operator+= (const iterator& other)
					{
						CheckConstraints_(other.m_count_);
						m_count_			+= other.m_count_;
						m_element_index_	= m_source_->CalculateIndex_(m_element_index_, other.m_element_index_);
					}

					/// <summary>Decrements the current element_index of the iterator by the value passed.</summary>
					/// <param name="decrement">The amount to subtract from the current element_index.</param>
					void operator-= (int decrement)
					{
						CheckConstraints_(-1 * decrement);
						m_count_			-= decrement;
						m_element_index_	= m_source_->CalculateIndex_(m_element_index_, -1 * decrement);
					}

					/// <summary>Decrements the current element_index of the iterator by the element_index of the passed iterator.</summary>
					/// <param name="other">Another iterator of the same type.</param>
					void operator-= (const iterator& other)
					{
						CheckConstraints_(-1 * other.m_count_);
						m_count_			-= other.m_count_;
						m_element_index_	= m_source_->CalculateIndex_(m_element_index_, -1 * other.m_element_index_);
					}

					/// <summary>Increments the current position of the iterator by one.</summary>
					/// <returns>An iterator pointing an element beyond the current.</returns>
					const iterator& operator++ (void)
					{
						CheckConstraints_(1);
						++m_count_;
						m_element_index_ = m_source_->CalculateIndex_(m_element_index_, 1);
						return *this;
					}

					/// <summary>Increments the current position of the iterator by one.</summary>
					/// <returns>An iterator pointing an element beyond the current.</returns>
					const iterator operator++ (int)
					{
						CheckConstraints_(1);
						iterator temporary = this;
						++temporary.m_count_;
						temporary.m_element_index_ = temporary.m_source_->CalculateIndex_(temporary.m_element_index_, 1);
						return temporary;
					}

					/// <summary>Decrements the current position of the iterator by one.</summary>
					/// <returns>An iterator pointing an element before the current.</returns>
					const iterator& operator-- (void)
					{
						CheckConstraints_(-1);
						iterator temporary = this;
						--m_count_;
						m_element_index_ = m_source_->CalculateIndex_(temporary.m_element_index_, -1);
						return *this;
					}

					/// <summary>Decrements the current position of the iterator by one.</summary>
					/// <returns>An iterator pointing an element before the current.</returns>
					const iterator operator-- (int)
					{
						CheckConstraints_(-1);
						iterator temporary = this;
						--temporary.m_count_;
						temporary.m_element_index_ = temporary.m_source_->CalculateIndex_(temporary.m_element_index_, -1);
						return temporary;
					}

					/// <summary>Compares the passed iterators position with the current.</summary>
					/// <param name="other">The iterator to compare with the current.</param>
					/// <returns>Whether or not the two iterators match position.</returns>
					bool operator== (const iterator& other) const
					{
						return (m_source_ == other.m_source_) && (m_count_ == other.m_count_);
					}

					/// <summary>Compares the passed iterators position with the current.</summary>
					/// <param name="other">The iterator to compare with the current.</param>
					/// <returns>Whether or not the two iterators point at different positions.</returns>
					bool operator!= (const iterator& other) const
					{
						return (m_source_ != other.m_source_) || (m_count_ != other.m_count_);
					}

					/// <summary>Compares the passed iterators position with the current.</summary>
					/// <param name="other">The iterator to compare with the current.</param>
					/// <returns>Whether or not the current position is larger compared to that of the passed iterator.</returns>
					bool operator> (const iterator& other) const
					{
						return (m_source_ == other.m_source_) && (m_count_ > other.m_count_);
					}

					/// <summary>Compares the passed iterators position with the current.</summary>
					/// <param name="other">The iterator to compare with the current.</param>
					/// <returns>Whether or not the current position is smaller compared to that of the passed iterator.</returns>
					bool operator< (const iterator& other) const
					{
						return (m_source_ == other.m_source_) && (m_count_ < other.m_count_);
					}

					/// <summary>Compares the passed iterators position with the current.</summary>
					/// <param name="other">The iterator to compare with the current.</param>
					/// <returns>Whether or not the current position is larger or equal compared to that of the passed iterator.</returns>
					bool operator>= (const iterator& other) const
					{
						return (m_source_ == other.m_source_) && (m_count_ >= other.m_count_);
					}

					/// <summary>Compares the passed iterators position with the current.</summary>
					/// <param name="other">The iterator to compare with the current.</param>
					/// <returns>Whether or not the current position is smaller or equal compared to that of the passed iterator.</returns>
					bool operator<= (const iterator& other) const
					{
						return (m_source_ == other.m_source_) && (m_count_ <= other.m_count_);
					}
			
				private:
					/// <summary>Checks if the iterator manipulation won't put it out of bounds in regards to the source buffer.</summary>
					/// <param name="modifier">The value modification for the count variable.</param>
					void CheckConstraints_(const int64_t modifier)
					{
						size_t new_count	= m_count_ + modifier;
						size_t difference	= new_count - m_count_;

						if (std::abs(modifier) > m_source_->m_elements_.size() ||
							m_count_ == size_t() - 1 ||
							difference != std::abs(modifier) ||
							new_count > m_source_->m_elements_.size())
						{
							throw std::runtime_error("CircularBuffer iterator has reached index constraints.");
						}
					}
			};
	};
}