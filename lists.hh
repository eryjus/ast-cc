/*
 *  ast-cc is an Abstract Syntax Tree compiler
 *  Copyright (C) 2014  Adam Clark
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __LIST_H__
#define __LIST_H__

template <class T>
class List {
private:
    T *_elem;
    List<T> *_nextList;

public:
    List(T *h, List<T> *n) : _elem(h), _nextList(n) {};

public:
    T *elem(void) const { return _elem; };
    List<T> *next(void) const { return _nextList; };
    void next(List<T> *n) { _nextList = n; }
};

// -- Templates for lists
//    -------------------
template <class T>
inline T *Append(T *l, T *r) {
	T *rv = l;
	if (!l) return r; if (!r) return l;
	while(l->next()) l = (T *)l->next();
	l->next(r);
	return rv;
}

template <class T>
inline T *First(T *l) {
	return (T*)l;
}

template <class T>
inline bool More(T *l) {
	return (l != 0);
}

template <class T>
inline T *Next(T *l) {
	if (More((T *)l)) return (T *)l->next();
	else return (T *)0;
}

template <class T>
inline int Len(T *i) {
	int rv; T *l;
	for (rv = 0, l = First((T *)i); More((T *)l); l = Next((T *)l)) rv ++;
	return rv;
}

template <class T>
inline T *Nth(T* l, int n) {
    if (n >= Len(l)) return (T *)0;
    for (; n > 0 && More(l); n --, l = Next(l)) { }
    return l;
}

#endif
