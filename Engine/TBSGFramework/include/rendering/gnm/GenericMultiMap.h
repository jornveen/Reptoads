#pragma once
#include "core/Assertion.h"
#ifdef PLATFORM_PS
#include <functional>
#include <unordered_map>
#include <functional>
#include "rendering/ResourceIds.h"
#include "memory/Containers.h"

namespace gfx
{
	template<typename BT>
	class GenericMultiMap
	{
	public:
		GenericMultiMap(std::function<Identifier(BT*)> m_idCompare);
		template<typename T>
		T& Get(Identifier a_rId) const;
		template<typename T>
		ptl::vector<T*> Get();
		template <class T>
		const ptl::vector<T*> GetConst() const;
		template <class T, class ... param>
		void Add(T* a_instance);
		template<typename T, typename... param>
		void Add(param&&... a_parameters);
		template<typename T>
		void OnItemAdded(std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)> a_func);
		void OnItemAdded(std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)> a_func);
		template<typename T>
		void OnItemRemoved(std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)> a_func);
		void OnItemRemoved(std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)> a_func);
		template <class T, class ... param>
		void Remove(T& a_instance);
		template <class T, class ... param>
		void Remove(Identifier a_id);
		template <class T>
		bool Contains(Identifier a_id);
		template <class T>
		bool Contains();
		void ClearMaps();
		void Execute(std::function<void(BT*)> a_func);
		template <class T>
		void Execute(std::function<void(T*)> a_func);

		ptl::vector<BT*> Get();
		const ptl::unordered_map<size_t, ptl::unordered_map<Identifier, BT*>>& GetRootMap();
		size_t GetTypeId(Identifier a_id);
		template<typename T>
		void DeleteAll();
		void DeleteAll();
	private:
		template<typename T>
		void HandleCallbacksAdd(T * a_instance);
		template<typename T>
		void HandleCallbacksRemove(T * a_instance);
		ptl::unordered_map<size_t, ptl::unordered_map<uint32_t, BT*>> m_items;
		std::function<Identifier(BT*)> m_idCompare;
		ptl::unordered_map<int, ptl::vector<std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)>>> m_onAdded;
		ptl::unordered_map<int, ptl::vector<std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)>>> m_onRemoved;
		ptl::unordered_map<size_t, std::function<void(void*)>> m_deleteFunctions;
	};

	template <typename BT>
	GenericMultiMap<BT>::GenericMultiMap(std::function<Identifier(BT*)> a_idCompare)
	{
		m_idCompare = a_idCompare;
	}

	// GET
	template <typename BT>
	template <typename T>
	T& GenericMultiMap<BT>::Get(Identifier a_rId) const
	{
		// check if the map exists
		auto map = m_items.find(T::id());
		if (map != m_items.end())
		{
			//existance check & return
			auto item = map->second.find(a_rId._id);
			if (item != map->second.end())
				return *static_cast<T*>(item->second);
		}

		// else throw exception
		ASSERT(false);
	}

	template <typename BT>
	template <typename T>
	ptl::vector<T*> GenericMultiMap<BT>::Get()
	{
		// check if the map exists
		auto map = m_items.find(T::id());
		ptl::vector<T*> vec;
		if (map != m_items.end())
		{
			//existance check & return
			for (auto item = map->second.begin(); item != map->second.end(); ++item)
				vec.push_back(static_cast<T*>(item->second));
			return vec;
		}

		// else return
		ASSERT(false);
	}

	template <typename BT>
	template <typename T>
	const ptl::vector<T*> GenericMultiMap<BT>::GetConst() const
	{
		// check if the map exists
		auto map = m_items.find(T::id());
		ptl::vector<T*> vec;
		if (map != m_items.end())
		{
			//existance check & return
			for (auto item = map->second.begin(); item != map->second.end(); item++)
				vec.push_back(static_cast<T*>(item->second));
			return vec;
		}

		// else add and return
		ASSERT(false);
	}

	// ADD BY INSTANCE
	template <typename BT>
	template <typename T, typename ... param>
	void GenericMultiMap<BT>::Add(T* a_instance)
	{
		// add if the map with the given type if it doesn't exist
		if (m_items.find(T::id()) == m_items.end())
		{
			m_items.emplace(T::id(), ptl::unordered_map<uint32_t, BT*>());
			m_deleteFunctions.emplace(T::id(), [&](void* x) {delete static_cast<T*>(x); x = nullptr; });
		}

		// check if the item already exists
		auto item = m_items.at(T::id()).find(m_idCompare(a_instance)._id);
		if (item != m_items.at(T::id()).end())
		{
			// THIS ITEM ALREADY EXISTS
			ASSERT(false);
		}

		// add the item
		m_items.at(T::id()).emplace(m_idCompare(a_instance)._id, a_instance);

		HandleCallbacksAdd<T>(a_instance);
	}

	// ADD BY ID
	template <typename BT>
	template <typename T, typename ... param>
	void GenericMultiMap<BT>::Add(param&&... a_parameters)
	{
		// add if the map with the given type if it doesn't exist
		if (m_items.find(T::id()) == m_items.end())
		{
			m_items.emplace(T::id(), ptl::unordered_map<uint32_t, BT*>());
			m_deleteFunctions.emplace(T::id(), [&](void* x) {delete static_cast<T*>(x); x = nullptr; });
		}

		T* x = new T{ std::forward<param>(a_parameters)... };

		// ITEM WITH ID EXISTS ALREADY
		auto item = m_items.at(T::id()).find(m_idCompare(x)._id);
		if (item != m_items.at(T::id()).end())
			ASSERT(false);

		// return the new item
		m_items.at(T::id()).emplace(m_idCompare(x)._id, x);

		HandleCallbacksAdd<T>(x);
	}

	template <typename BT>
	template<typename T>
	void GenericMultiMap<BT>::HandleCallbacksAdd(T* a_instance)
	{
		//call all type specific callbacks
		if (m_onAdded.find(T::id()) != m_onAdded.end())
			for (auto x : m_onAdded.find(T::id())->second)
				x(static_cast<typename std::add_pointer<typename std::remove_const<BT>::type>::type>(a_instance));

		//call all general callbacks
		if (m_onAdded.find(-1) != m_onAdded.end())
			for (int i = 0; i < m_onAdded.at(-1).size(); i++)
				m_onAdded.at(-1)[i](static_cast<typename std::add_pointer<typename std::remove_const<BT>::type>::type>(a_instance));
	}

	template <typename BT>
	template<typename T>
	void GenericMultiMap<BT>::HandleCallbacksRemove(T* a_instance)
	{
		//call all type specific callbacks
		if (m_onRemoved.find(T::id()) != m_onRemoved.end())
			for (auto x : m_onRemoved.find(T::id())->second)
				x(static_cast<typename std::add_pointer<typename std::remove_const<BT>::type>::type>(a_instance));

		//call all general callbacks
		if (m_onRemoved.find(-1) != m_onRemoved.end())
			for (int i = 0; i < m_onRemoved.at(-1).size(); i++)
				m_onRemoved.at(-1)[i](static_cast<typename std::add_pointer<typename std::remove_const<BT>::type>::type>(a_instance));
	}

	template <typename BT>
	template <typename T>
	void GenericMultiMap<BT>::OnItemAdded(std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)> a_func)
	{
		auto list = m_onAdded.find(T::id());
		if (list == m_onAdded.end())
		{
			m_onAdded.emplace(T::id(), ptl::vector<std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)>>());
			m_onAdded.find(T::id())->second.push_back(a_func);
			return;
		}

		list->second.push_back(a_func);
	}

	template <typename BT>
	void GenericMultiMap<BT>::OnItemAdded(std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)> a_func)
	{
		auto list = m_onAdded.find(-1);
		if (list == m_onAdded.end())
		{
			m_onAdded.emplace(-1, ptl::vector<std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)>>());
			m_onAdded.find(-1)->second.push_back(a_func);
			return;
		}

		list->second.push_back(a_func);
	}

	template <typename BT>
	template <typename T>
	void GenericMultiMap<BT>::OnItemRemoved(std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)> a_func)
	{
		auto list = m_onRemoved.find(T::id());
		if (list == m_onRemoved.end())
		{
			m_onRemoved.emplace(T::id(), ptl::vector<std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)>>());
			m_onRemoved.find(T::id())->second.push_back(a_func);
			return;
		}

		list->second.push_back(a_func);
	}

	template <typename BT>
	void GenericMultiMap<BT>::OnItemRemoved(std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)> a_func)
	{
		auto list = m_onRemoved.find(-1);
		if (list == m_onRemoved.end())
		{
			m_onRemoved.emplace(-1, ptl::vector<std::function<void(typename std::add_pointer<typename std::remove_const<BT>::type>::type)>>());
			m_onRemoved.find(-1)->second.push_back(a_func);
			return;
		}

		list->second.push_back(a_func);
	}

	template <typename BT>
	template <class T, class ... param>
	void GenericMultiMap<BT>::Remove(T& a_instance)
	{
		// crash if not found
		Get<T>(m_idCompare(&a_instance));
		m_items.at(T::id()).erase(m_idCompare(&a_instance)._id);
		HandleCallbacksRemove<T>(&a_instance);
	}

	template <typename BT>
	template <class T, class ... param>
	void GenericMultiMap<BT>::Remove(Identifier a_id)
	{
		// crash if not found
		T* x = &Get<T>(a_id);
		m_items.at(T::id()).erase(a_id._id);
		HandleCallbacksRemove<T>(x);
	}

	template <typename BT>
	template <typename T>
	bool GenericMultiMap<BT>::Contains(Identifier a_id)
	{
		// add if the map with the given type if it doesn't exist
		if (m_items.find(T::id()) == m_items.end())
			return false;

		// check if the item already exists
		auto item = m_items.at(T::id()).find(a_id._id);
		if (item != m_items.at(T::id()).end())
			return true;

		return false;
	}

	template <typename BT>
	template <typename T>
	bool GenericMultiMap<BT>::Contains()
	{
		// add if the map with the given type if it doesn't exist
		if (m_items.find(T::id()) == m_items.end())
			return false;

		if (m_items.at(T::id()).size() == 0)
			return false;

		return true;
	}

	template <typename BT>
	void GenericMultiMap<BT>::ClearMaps()
	{
		m_items.clear();
	}

	template <typename BT>
	void GenericMultiMap<BT>::Execute(std::function<void(BT*)> a_func)
	{
		for (auto m = m_items.begin(); m != m_items.end(); ++m)
		{
			if (m == m_items.end())
				return;

			for (auto x = m->second.begin(); x != m->second.end(); ++x)
				a_func(x->second);
		}
	}

	template <typename BT>
	template <typename T>
	void GenericMultiMap<BT>::Execute(std::function<void(T*)> a_func)
	{
		for (auto entry = m_items.at(T::id()).begin(); entry != m_items.at(T::id()).end(); ++entry)
			a_func((T*)entry->second);
	}

	template <typename BT>
	ptl::vector<BT*> GenericMultiMap<BT>::Get()
	{
		ptl::vector<BT*> vec;
		for (auto m = m_items.begin(); m != m_items.end(); ++m)
		{
			for (auto item = m->second.begin(); item != m->second.end(); item++)
				vec.push_back((item->second));
		}

		return vec;
	}

	template <typename BT>
	const ptl::unordered_map<size_t, ptl::unordered_map<Identifier, BT*>>& GenericMultiMap<BT>::GetRootMap()
	{
		return m_items;
	}

	template <typename BT>
	size_t GenericMultiMap<BT>::GetTypeId(Identifier a_id)
	{
		for (auto m = m_items.begin(); m != m_items.end(); ++m)
		{
			// check if the item already exists
			auto item = m->second.find(a_id._id);
			if (item != m->second.end())
				return m->first;
		}

		// the item with this ID doesn't exist
		ASSERT(false);
	}

	template <typename BT>
	template <typename T>
	void GenericMultiMap<BT>::DeleteAll() {
		auto& func = m_deleteFunctions.at(T::id());
		for (auto entry = m_items.at(T::id()).begin(); entry != m_items.at(T::id()).end(); ++entry)
			func(entry->second);

		m_items.at(T::id()).clear();
	}

	template <typename BT>
	void GenericMultiMap<BT>::DeleteAll()
	{
		for (auto& list : m_items)
		{
			auto& func = m_deleteFunctions.at(list.first);
			for (auto& item : list.second)
			{
				func(item.second);
			}
		}
		m_items.clear();
	}

}
#endif