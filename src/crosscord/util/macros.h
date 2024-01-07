#pragma once

#define DECLARE_SINGLETON(ClassName) private: \
	static ClassName* _Instance;\
public:\
	ClassName(const ClassName&) = delete;\
	static ClassName* Get() {\
		if (!_Instance)\
			_Instance = new ClassName();\
		return _Instance;\
	}

#define INITIALIZE_SINGLETON(ClassName) ClassName* ClassName::_Instance = nullptr