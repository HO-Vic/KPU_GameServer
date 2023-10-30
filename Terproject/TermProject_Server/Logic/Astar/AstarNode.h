#pragma once
#include "../../PCH/stdafx.h"
class AstarNode
{
private:
	float m_distance;
	std::pair<short, short> m_myNode;
	std::pair<short, short> m_parentNode;
public:
	AstarNode();
	AstarNode(std::pair<short, short>& myNode, std::pair<short, short>& parentNode, float distance);
	AstarNode(AstarNode& other)
	{
		m_distance = other.m_distance;
		m_myNode = other.m_myNode;
		m_parentNode = other.m_parentNode;
	}
	AstarNode(AstarNode&& other) noexcept
	{
		m_distance = other.m_distance;
		m_myNode = other.m_myNode;
		m_parentNode = other.m_parentNode;
	}
public:
	void ModifyDistance(float distance, std::pair<short, short>& parentNode);
	void ModifyDistance(float distance, short parentNodeX, short parentNodeY);

	bool operator==(const AstarNode& L) const
	{
		if (abs(m_distance - L.m_distance) > DBL_EPSILON) return false;
		if (m_myNode != L.m_myNode) return false;
		if (m_parentNode != L.m_parentNode) return false;
		return true;
	}

	constexpr bool operator < (const AstarNode& L) const
	{
		return (m_distance < L.m_distance);
	}

	AstarNode& operator= (const AstarNode& other)
	{
		m_distance = other.m_distance;
		m_myNode = other.m_myNode;
		m_parentNode = other.m_parentNode;
		return *this;
	}

	AstarNode& operator= (const AstarNode&& other) noexcept
	{
		m_distance = other.m_distance;
		m_myNode = other.m_myNode;
		m_parentNode = other.m_parentNode;
		return *this;
	}

public:
	float GetDistance();
	std::pair<short, short> GetNodePos();
	std::pair<short, short> GetParentNodePos();
};

