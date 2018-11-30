#include "IO/BufferedWriter.h"

#include <stdexcept>

namespace TAPP::IO
{
	/*** Constructors ***********************************************************/

	BufferedWriter::BufferedWriter(size_t buffer_size) : m_maximum_buffer_size_(buffer_size)
	{

	}

	BufferedWriter::~BufferedWriter(void)
	{
		CloseFile();
	}

	/*** Public Functions *******************************************************/

	void BufferedWriter::CloseFile(void)
	{
		m_mutex_.lock();
		if (m_writer_.is_open())
		{
			WriteBuffer_();
			m_writer_.close();
		}
		m_mutex_.unlock();
	}

	bool BufferedWriter::IsFileOpen(void)
	{
		m_mutex_.lock();
		bool is_open = m_writer_.is_open();
		m_mutex_.unlock();

		return is_open;
	}

	void BufferedWriter::OpenFile(const std::string& filepath)
	{
		if (m_writer_.is_open())
		{
			CloseFile();
		}
		m_mutex_.lock();
		m_writer_.open(filepath);
		m_buffer_ = "";
		m_buffer_.reserve(m_maximum_buffer_size_);
		m_mutex_.unlock();
	}

	void BufferedWriter::Write(const std::string& text)
	{
		AddToBuffer_(text);
	}

	BufferedWriter& BufferedWriter::operator<<(const std::string& text)
	{
		AddToBuffer_(text);
		return *this;
	}

	/*** Private Functions ******************************************************/

	void BufferedWriter::AddToBuffer_(const std::string& text)
	{
		m_mutex_.lock();
		if (m_writer_.is_open())
		{
			if (m_buffer_.size() + text.size() >= m_maximum_buffer_size_)
			{
				WriteBuffer_();
			}

			m_buffer_ += text;
		}
		else
		{
			m_mutex_.unlock();
			throw std::runtime_error("BufferedWriter currently doesn't have an open file.");
		}

		m_mutex_.unlock();
	}

	void BufferedWriter::WriteBuffer_(void)
	{
		m_writer_ << m_buffer_;
		m_buffer_ = "";
	}
}
