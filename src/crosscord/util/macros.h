#pragma once


#define DECLARE_SINGLETON(ClassName) private: \
	static ClassName* _Instance; \
public: \
	ClassName(const ClassName&) = delete; \
	static ClassName* Get() { return _Instance; }

#define INSTANTIATE_SINGLETON(ClassName) ClassName* ClassName::_Instance = new ClassName();