// Map.h

#ifndef MAP_H
#define MAP_H

  // Later in the course, we'll see that templates provide a much nicer
  // way of enabling us to have Maps of different types.  For now, we'll
  // use type aliases.

#include <string>

using KeyType = std::string;
using ValueType = double;

class Map
{
public:
	Map(); // Create an empty map (i.e., one with no key/value pairs)

	bool empty() const;  // Return true if the map is empty, otherwise false.

	int size() const;    // Return the number of key/value pairs in the map.

	bool insert(const KeyType& key, const ValueType& value);
	// If key is not equal to any key currently in the map, and if the
	// key/value pair can be added to the map, then do so and return true.
	// Otherwise, make no change to the map and return false (indicating
	// that either the key is already in the map, or the map has a fixed
	// capacity and is full.

	bool update(const KeyType& key, const ValueType& value);
	// If key is equal to a key currently in the map, then make that key no
	// longer map to the value it currently maps to, but instead map to
	// the value of the second parameter; return true in this case.
	// Otherwise, make no change to the map and return false.

	bool insertOrUpdate(const KeyType& key, const ValueType& value);
	// If key is equal to a key currently in the map, then make that key no
	// longer map to the value that it currently maps to, but instead map to
	// the value of the second parameter; return true in this case.
	// If key is not equal to any key currently in the map, and if the
	// key/value pair can be added to the map, then do so and return true.
	// Otherwise, make no change to the map and return false (indicating
	// that the key is not already in the map and the map has a fixed
	// capacity and is full.

	bool erase(const KeyType& key);
	// If key is equal to a key currently in the map, remove the key/value
	// pair with that key from the map and return true.  Otherwise, make
	// no change to the map and return false.

	bool contains(const KeyType& key) const;
	// Return true if key is equal to a key currently in the map, otherwise
	// false.

	bool get(const KeyType& key, ValueType& value) const;
	// If key is equal to a key currently in the map, set value to the
	// value in the map which that key maps to and return true.  Otherwise,
	// make no change to the value parameter of this function and return
	// false.

	bool get(int i, KeyType& key, ValueType& value) const;
	// If 0 <= i < size(), copy into the key and value parameters the
	// key and value of one of the key/value pairs in the map and return
	// true.  Otherwise, leave the key and value parameters unchanged and
	// return false.

	void swap(Map& other);
	// Exchange the contents of this map with the other one.

	// Housekeeping functions
	~Map();
	Map(const Map& other);
	Map& operator=(const Map& rhs);

private:
	// Since this structure is used only by the implementation of the
	 // Map class, we'll make it private to Map.

	struct Pair
	{
		KeyType   m_key;
		ValueType m_value;
		Pair* m_next;
		Pair* m_prev;
	};

	// This is a circular doubly-linked list with a dummy node
	
	Pair* m_head = new Pair;
	// Pointer m_head pointing to dummy node notifies the beginning of the linked list.

	bool doInsertOrUpdate(const KeyType& key, const ValueType& value, bool mayInsert, bool mayUpdate);
	// If the key is not present in the map and if mayInsert is true, insert
	// the pair if there is room.  If the key is present and mayUpdate is
	// true, update the pair with the given key.

	Pair* find(const KeyType& key) const
	// Return pointer to the pair in linked list whose m_key == key if there is
	// one, else nullptr
	{
		// Do a linear search through the linked list.
		for (Pair* ptr = m_head->m_next; ptr != m_head; ptr = ptr->m_next)
			if ((*ptr).m_key == key)
				return ptr;
		return nullptr;
	}
};


// Inline implementations

inline
bool Map::empty() const
{
	return size() == 0;
}

inline
bool Map::contains(const KeyType& key) const
{
	return find(key) != nullptr;
}

inline
bool Map::insert(const KeyType& key, const ValueType& value)
{
	return doInsertOrUpdate(key, value, true, false);
}

inline
bool Map::update(const KeyType& key, const ValueType& value)
{
	return doInsertOrUpdate(key, value, false, true);
}

inline
bool Map::insertOrUpdate(const KeyType& key, const ValueType& value)
{
	return doInsertOrUpdate(key, value, true, true);
}

// non-member functions (some map algorithms)

bool combine(const Map& m1, const Map& m2, Map& result);
void reassign(const Map& m, Map& result);

#endif // for MAP_H