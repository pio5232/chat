#pragma once

enum class ComponentType
{
	NONE,
	Transform,
	Stat,
	Move,
};

class BaseComponent
{
public:
	BaseComponent(ULONGLONG userId, ComponentType componentType);
	virtual ~BaseComponent();

private:
	ULONGLONG _userId = 0;
	ComponentType _componentType = ComponentType::NONE;

};

