#pragma once

#ifndef IL2CPP_INCLUDES
#define IL2CPP_INCLUDES

#include <obfuscate.h>
#include <Logger.h>
#include "Unity/Unity.h"
#include "Unity/Matrix4x4.hpp"

#define getFieldOffset(method) Il2Cpp::GetFieldOffset(getAssembly(), getNamespace(), getClass(), OBFUSCATE(method))
#define getMethodOffset(method, param) Il2Cpp::GetMethodOffset(getAssembly(), getNamespace(), getClass(), OBFUSCATE(method), param)
#define getFieldStaticVal(method, out) Il2Cpp::GetStaticFieldValue(getAssembly(), getNamespace(), getClass(), OBFUSCATE(method),out)
#define setPointer(func, method, param) func = Il2Cpp::GetMethod<decltype(func)>(getAssembly(), getNamespace(), getClass(), OBFUSCATE(method),param)

class CObject;

class CRenderer;

class CComponent;

class CTransform;

class CGameObject;

class CCamera;

class CActorLinker;

class CActorCreators;

class CGamePlay;

class CActorHelper;

class CValueLinkerComponent;

#include "Object.hpp"
#include "Component.hpp"
#include "Renderer.hpp"
#include "Transform.hpp"
#include "GameObject.hpp"
#include "Camera.hpp"
#include "Screen.hpp"
#include "GamePlay.hpp"
#include "ActorLinker.hpp"
#include "ActorCreators.hpp"
#include "ActorHelper.hpp"
#include "ValueLinkerComponent.hpp"

namespace Il2Cpp {

    //Initialize your classes here
    inline void InitClass() {

        Object::Initialize();
        Component::Initialize();
        Renderer::Initialize();
        Transform::Initialize();
        GameObject::Initialize();
        Camera::Initialize();
        Screen::Initialize();
        GamePlay::Initialize();
        ActorCreators::Initialize();
        ActorLinker::Initialize();
        ActorHelper::Initialize();
        ValueLinkerComponent::Initialize();
        LOGI("libil2cpp.so completed!");
        IsReady = true;
    }
}

#endif