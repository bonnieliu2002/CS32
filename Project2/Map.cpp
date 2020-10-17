#include "Map.h"

Map::Map()
{
	// initialize Map to be an empty linked list
	// it only has a dummy node, which means its next and previous pointers as well as the head pointer all point to itself
	m_head->m_next = m_head->m_prev = m_head;
}

Map::Map(const Map& other)
{
	// copy constructor for Map
	// initially, it only has a dummy node, which means its next and previous pointers as well as the head pointer all point to itself
	m_head->m_next = m_head->m_prev = m_head;
	for (Pair* ptr = other.m_head->m_next; ptr != other.m_head; ptr = ptr->m_next)
		insert(ptr->m_key, ptr->m_value);
}

Map& Map::operator=	(const Map& rhs)
{
	// check if right-hand side and left-hand side are the same
	if (&rhs == this)
		return *this;
	// assignment operator for Map
	Pair* ptr = m_head->m_next;
	while (ptr != m_head)
	{
		ptr = ptr->m_next;
		delete ptr->m_prev;
	}
	// now Map only has a dummy node, which means its next and previous pointers as well as the head pointer all point to itself
	m_head->m_next = m_head->m_prev = m_head;
	// insert copies of rhs key/value pairs into current Map without directly copying its pointers
	for (Pair* ptr = rhs.m_head->m_next; ptr != rhs.m_head; ptr = ptr->m_next)
		insert(ptr->m_key, ptr->m_value);
	return *this;
}

Map::~Map()
{
	// delete all the "real" nodes
	Pair* ptr = m_head->m_next;
	while (ptr != m_head)
	{
		ptr = ptr->m_next;
		delete ptr->m_prev;
	}
	// delete dummy node
	delete m_head;
}

int Map::size() const
{
	// use pointer ptr to traverse through linked list and count how many "real" nodes there are
	int count = 0;
	for (Pair* ptr = m_head->m_next; ptr != m_head; ptr = ptr->m_next)
		count++;
	return count;
}

bool Map::erase(const KeyType& key)
{
	Pair* ptr = find(key);
	if (ptr == nullptr) // key not found
		return false;
	// key found, so change next and previous pointers and delete pair
	ptr->m_prev->m_next = ptr->m_next;
	ptr->m_next->m_prev = ptr->m_prev;
	delete ptr;
	return true;
}

bool Map::get(const KeyType& key, ValueType& value) const
{
	Pair* ptr = find(key);
	if (ptr == nullptr) // key not found, so leave value unchanged
		return false;
	value = ptr->m_value; // key found, so update value parameter
	return true;
}

bool Map::get(int i, KeyType& key, ValueType& value) const
{
	// if parameter i tries to access pair out of bounds, return false
	if (i < 0 || i >= size())
		return false;
	// go through linked list indexed {0, 1, ..., i - 1} until ith element is reached
	Pair* ptr = m_head->m_next;
	for (int j = 0; j < i; j++)
		ptr = ptr->m_next;
	// copy desired pair's key and value into key and value parameters
	key = ptr->m_key;
	value = ptr->m_value;
	return true;
}

void Map::swap(Map& other)
{
	// swap head pointers
	Pair* ptr = m_head;
	m_head = other.m_head;
	other.m_head = ptr;
}

bool Map::doInsertOrUpdate(const KeyType& key, const ValueType& value, bool mayInsert, bool mayUpdate)
{
	Pair* pos = find(key);
	if (pos != nullptr) { // key has been found
		if (mayUpdate)
			pos->m_value = value;
		return mayUpdate;
	}
	// no key found, so we must insert
	if (!mayInsert)
		return false;
	Pair* ptr = new Pair;
	// assign key and value
	ptr->m_key = key;
	ptr->m_value = value;
	// reconnect pointers of linked list
	ptr->m_next = m_head;
	ptr->m_prev = m_head->m_prev;
	ptr->m_prev->m_next = ptr;
	m_head->m_prev = ptr;
	return true;
}

bool combine(const Map& m1, const Map& m2, Map& result) {
	if (&result != &m1 && &result != &m2)
	{
		// clear result Map
		Map empty;
		result = empty;
	}
	// matched describes whether there exists a key in the two maps with different corresponding values
	bool matched = true;
	// declare variables to use in get functions
	KeyType key1, key2;
	ValueType value1, value2;
	// add all items from m1 into result
	for (int i = 0; i < m1.size(); i++)
	{
		m1.get(i, key1, value1);
		if (!result.insert(key1, value1) && result.get(key1, value2) && value2 != value1)
		{
			matched = false;
			result.erase(key1);
		}
	}
	// attempt to add items from m2 into result
	for (int j = 0; j < m2.size(); j++)
	{
		m2.get(j, key2, value2);
		// if key is already in result and if corresponding values are different, erase key from result
		if (!result.insert(key2, value2) && result.get(key2, value1) && value1 != value2)
		{
			matched = false;
			result.erase(key2);
		}
	}
	return matched;
}

void reassign(const Map& m, Map& result)
{
	if (&m != &result)
	{
		// clear result Map if they are not the same Map
		Map empty;
		result = empty;
	}
	// declare temporary variables to hold our values that we will receive from our get functions
	KeyType key1, key2, temp_key;
	ValueType value1, value2, temp_value;
	// store value of first node
	m.get(0, key1, temp_value);
	// store key of last node
	m.get(m.size() - 1, temp_key, value2);
	// traverse the loop
	for (int i = 0; i < m.size(); i++)
	{
		m.get(i, key1, value1);
		m.get(i + 1, key2, value2);
		// if result and m are the same Map, result will be updated; otherwise, insert key1 and value2 into result
		result.insertOrUpdate(key1, value2);
	}
	// update last node so that its value is that of the first node's original value
	result.update(temp_key, temp_value);
}