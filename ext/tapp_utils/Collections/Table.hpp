#pragma once
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

namespace TAPP::Collections
{
	template <typename T>
	class Table : public std::unordered_map<size_t, T>
	{
		public:
			typedef std::function<std::vector<std::string>(std::vector<T*>&, const std::string&)> SelectionToStringFunction; // std::function<std::vector<std::string>(T&, std::vector<std::string>&)>
			typedef std::function<bool(T*)> SelectQuery; // std::function<bool(T&)>

			Table(void)
			{
			}

			Table(std::string table_name, std::vector<std::string> column_names, SelectionToStringFunction to_string_function) 
				: m_column_names_(column_names), m_table_name_(table_name), m_to_string_(to_string_function)
			{
			}

			Table(Table&& other) : std::unordered_map<size_t, T>(std::move(other)), m_column_names_(std::move(other.m_column_names_)), m_table_name_(std::move(other.m_table_name_)), m_to_string_(std::move(other.m_to_string_))
			{
			}

			Table& operator=(Table&& other)
			{
				std::unordered_map<size_t, T>::swap(other);
				m_column_names_.swap(other.m_column_names_);
				m_table_name_.swap(other.m_table_name_);
				m_to_string_.swap(other.m_to_string_);

				return *this;
			}

			~Table(void)
			{

			}

			Table(const Table& other) = delete;
			Table<T>& operator=(const Table<T>& other) = delete;

			void swap(Table<T>&& other)
			{
				std::unordered_map<size_t, T>::swap(other);
				m_column_names_.swap(other.m_column_names_);
				m_table_name_.swap(other.m_table_name_);
				m_to_string_.swap(other.m_to_string_);
			}

			std::vector<std::string> GetColumnNames(void)
			{
				return m_column_names_;
			}

			std::string GetTableName(void)
			{
				return m_table_name_;
			}

			std::vector<T*> Select(void)
			{
				return MapToVector_();
			}

			std::vector<T*> Select(SelectQuery query)
			{
				std::vector<T*> records(MapToVector_());
				records.erase(records.begin(), records.end(), std::not1(std::remove_if(query)));
				return records;
			}

			std::vector<std::string> RecordsToString()
			{
				return m_to_string_(MapToVector_(), std::vector<std::string>());
			}

			std::vector<std::string> RecordsToString(std::string& column_name)
			{
				return m_to_string_(MapToVector_(), column_name);
			}

			std::vector<std::string> RecordsToString(std::vector<T*>& records, const std::string& column_name)
			{
				return m_to_string_(records, column_name);
			}

			std::pair<typename std::unordered_map<size_t, T>::iterator, bool> insert(T object)
			{
				return std::unordered_map<size_t, T>::insert({ std::unordered_map<size_t, T>::size() + 1, object });
			}

			std::pair<typename std::unordered_map<size_t, T>::iterator, bool> insert(size_t index, T object)
			{
				return std::unordered_map<size_t, T>::insert({ index, object });
			}

		private:
			std::vector<std::string>		m_column_names_;
			std::string						m_table_name_;
			SelectionToStringFunction		m_to_string_;

			inline std::vector<T*> MapToVector_(void)
			{
				std::vector<T*> vector_from_map;
				vector_from_map.reserve(std::unordered_map<size_t, T>::size());
				for (typename std::unordered_map<size_t, T>::iterator it = std::unordered_map<size_t, T>::begin(); it != std::unordered_map<size_t, T>::end(); ++it)
				{
					vector_from_map.push_back(&it->second);
				}
				return vector_from_map;
			}
	};
}