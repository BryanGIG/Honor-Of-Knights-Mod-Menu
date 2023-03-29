#pragma once

struct RaycastHit {
    Vector3 Point, Normal;
    std::uint32_t FaceID;
    float Distance;
    Vector2 UV;
    int32_t m_Collider;
};
