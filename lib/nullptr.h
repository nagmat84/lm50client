#ifndef _nullptr_h_
#define _nullptr_h_

const class {
	public:
		template<class T> operator T*() const { return 0; }
		template<class C, class T> operator T C::*() const { return 0; }
	private:
		void operator&() const;
} nullptr = {};  

#endif
