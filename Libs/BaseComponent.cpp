#include "LibsPch.h"
#include "BaseComponent.h"

BaseComponent::BaseComponent(ULONGLONG userId, ComponentType componentType) : _userId(userId), _componentType(componentType)
{
}

BaseComponent::~BaseComponent()
{
}
