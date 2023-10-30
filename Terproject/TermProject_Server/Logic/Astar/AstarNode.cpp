#include "stdafx.h"
#include "AstarNode.h"

AstarNode::AstarNode() :m_distance(0.0f), m_myNode(0, 0), m_parentNode(0, 0)
{
}

AstarNode::AstarNode(std::pair<short, short>& myNode, std::pair<short, short>& parentNode, float distance) :m_distance(distance), m_myNode(myNode), m_parentNode(parentNode)
{
}

void AstarNode::ModifyDistance(float distance, std::pair<short, short>& parentNode)
{
	if (m_distance > distance) {
		m_distance = distance;
		m_parentNode = parentNode;
	}
}

void AstarNode::ModifyDistance(float distance, short parentNodeX, short parentNodeY)
{
	if (m_distance > distance) {
		m_distance = distance;
		m_parentNode = make_pair(parentNodeX, parentNodeY);
	}
}

float AstarNode::GetDistance()
{
	return m_distance;
}

std::pair<short, short> AstarNode::GetNodePos()
{
	return m_myNode;
}

std::pair<short, short> AstarNode::GetParentNodePos()
{
	return m_parentNode;
}
