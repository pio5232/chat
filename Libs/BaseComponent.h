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
	BaseComponent(ComponentType componentType);
	virtual ~BaseComponent();

	virtual void Update(float delta) = 0;
	ComponentType GetType() const { return _componentType; }
private:
	ComponentType _componentType = ComponentType::NONE;
	//ULONGLONG _userId = 0;

};

