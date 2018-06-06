/*
 * PrecomputeMap.cpp
 *
 * Copyright (c) 2014-2015, Steve Rabin
 * All rights reserved.
 *
 * An explanation of the JPS+ algorithm is contained in Chapter 14
 * of the book Game AI Pro 2, edited by Steve Rabin, CRC Press, 2015.
 * A presentation on Goal Bounding titled "JPS+: Over 100x Faster than A*"
 * can be found at www.gdcvault.com from the 2015 GDC AI Summit.
 * A copy of this code is on the website http://www.gameaipro.com.
 *
 * If you develop a way to improve this code or make it faster, please
 * contact steve.rabin@gmail.com and share your insights. I would
 * be equally eager to hear from anyone integrating this code or using
 * the Goal Bounding concept in a commercial application or game.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STEVE RABIN ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "stdafx.h"
#include "PrecomputeMap.h"
#include "JPSPlus.h"
#include "Log.h"
#include <fstream>

using std::ifstream;
using std::ofstream;

//#define FILE_FORMAT_ASCII
#define INVALID_GOAL_BOUNDS -1

PrecomputeMap::PrecomputeMap(int width, int height, std::vector<bool> map)
	: m_mapCreated(false), m_width(width), m_height(height), m_map(map)
{
}

PrecomputeMap::~PrecomputeMap()
{
}

DistantJumpPoints** PrecomputeMap::CalculateMap()
{
	m_mapCreated = true;

	// Calculate primary jump points.
	InitArray(m_jumpPointMap, m_width, m_height);
	CalculateJumpPointMap();

	// Calculate cardinal and diagnal jump points.
	InitArray(m_distantJumpPointMap, m_width, m_height);
	CalculateDistantJumpPointMap();

	// Destroy the m_jumpPointMap since it isn't needed for the search
	DestroyArray(m_jumpPointMap);

	return m_distantJumpPointMap;
}

void PrecomputeMap::SaveMap(const char *filename)
{
#ifdef FILE_FORMAT_ASCII
	ofstream file(filename);

	for (int r = 0; r < m_height; r++)
	{
		for (int c = 0; c < m_width; c++)
		{
			DistantJumpPoints* jumpPoints = &m_distantJumpPointMap[r][c];

			for (int i = 0; i < 8; i++)
			{
				file << (int)jumpPoints->jumpDistance[i] << "\t";
			}
			for (int dir = 0; dir < 8; dir++)
			{
				for (int minMaxIndex = 0; minMaxIndex < 4; minMaxIndex++)
				{
					file << (int)m_goalBoundsMap[r][c].bounds[dir][minMaxIndex] << "\t";
				}
			}
			file << std::endl;
		}
		file << std::endl;
	}
#else
	ofstream file(filename, std::ios::out | std::ios::binary);

	for (int r = 0; r < m_height; r++)
	{
		for (int c = 0; c < m_width; c++)
		{
			if (IsWall(r, c))
			{
				// Don't save data if a wall
				continue;
			}

			// Save Jump Distances
			DistantJumpPoints* jumpPoints = &m_distantJumpPointMap[r][c];

			for (int i = 0; i < 8; i++)
			{
				file.write((char*)&jumpPoints->jumpDistance[i], 2);
			}
		}
	}
#endif
}

void PrecomputeMap::LoadMap(const char *filename)
{
	m_mapCreated = true;

#ifdef FILE_FORMAT_ASCII
	ifstream file(filename, std::ios::in);

	InitArray(m_jumpDistancesAndGoalBoundsMap, m_width, m_height);

	for (int r = 0; r < m_height; r++)
	{
		for (int c = 0; c < m_width; c++)
		{
			JumpDistancesAndGoalBounds* map = &m_jumpDistancesAndGoalBoundsMap[r][c];
			map->blockedDirectionBitfield = 0;

			for (int i = 0; i < 8; i++)
			{
				file >> map->jumpDistance[i];
			}
			for (int i = 0; i < 8; i++)
			{
				// Detect invalid movement from jump distances
				// (jump distance of zero is invalid movement)
				if (map->jumpDistance[i] == 0)
				{
					map->blockedDirectionBitfield |= (1 << i);
				}
			}
			for (int dir = 0; dir < 8; dir++)
			{
				file >> m_jumpDistancesAndGoalBoundsMap[r][c].bounds[dir][MinRow];
				file >> m_jumpDistancesAndGoalBoundsMap[r][c].bounds[dir][MaxRow];
				file >> m_jumpDistancesAndGoalBoundsMap[r][c].bounds[dir][MinCol];
				file >> m_jumpDistancesAndGoalBoundsMap[r][c].bounds[dir][MaxCol];
			}

		}
	}
#else
	ifstream file(filename, std::ios::in | std::ios::binary);

	InitArray(m_jumpDistancesAndGoalBoundsMap, m_width, m_height);

	for (int r = 0; r < m_height; r++)
	{
		for (int c = 0; c < m_width; c++)
		{
			if (IsWall(r, c))
			{
				// Don't load data if a wall
				continue;
			}

			JumpDistancesAndGoalBounds* map = &m_jumpDistancesAndGoalBoundsMap[r][c];
			map->blockedDirectionBitfield = 0;

			// Load Jump Distances
			for (int i = 0; i < 8; i++)
			{
				file.read((char*)&map->jumpDistance[i], 2);
			}

			// Fabricate wall bitfield for each node
			for (int i = 0; i < 8; i++)
			{
				// Jump distance of zero is invalid movement and means a wall
				if (map->jumpDistance[i] == 0)
				{
					map->blockedDirectionBitfield |= (1 << i);
				}
			}
		}
	}
#endif
}

void PrecomputeMap::ConstrutMap()
{
	m_mapCreated = true;

	InitArray(m_jumpDistancesAndGoalBoundsMap, m_width, m_height);

	for (int r = 0; r < m_height; r++)
	{
		for (int c = 0; c < m_width; c++)
		{
			if (IsWall(r, c))
			{
				// Don't load data if a wall
				continue;
			}

			JumpDistancesAndGoalBounds* map = &m_jumpDistancesAndGoalBoundsMap[r][c];
			map->blockedDirectionBitfield = 0;

            // Init jumpDistance for each points.
			DistantJumpPoints* jumpPoints = &m_distantJumpPointMap[r][c];
			for (int i = 0; i < 8; i++)
			{
				map->jumpDistance[i] = jumpPoints->jumpDistance[i];
			}

			// Fabricate wall bitfield for each node
			for (int i = 0; i < 8; i++)
			{
				// Jump distance of zero is invalid movement and means a wall
				if (map->jumpDistance[i] == 0)
				{
					map->blockedDirectionBitfield |= (1 << i);
				}
			}
		}
	}
}

template <typename T>
void PrecomputeMap::InitArray(T**& t, int width, int height)
{
	t = new T*[height];
	for (int i = 0; i < height; ++i)
	{
		t[i] = new T[width];
		memset(t[i], 0, sizeof(T)*width);
	}
}

template <typename T>
void PrecomputeMap::DestroyArray(T**& t)
{
	for (int i = 0; i < m_height; ++i)
		delete[] t[i];
	delete[] t;

	t = 0;
}

//void PrecomputeMap::CalculateJumpPointMap()
//{
//	for (int r = 0; r < m_height; ++r)
//	{
//		for (int c = 0; c < m_width; ++c)
//		{
//			if (m_map[c + (r * m_width)])
//			{
//				if (IsJumpPoint(r, c, 1, 0))
//				{
//					m_jumpPointMap[r][c] |= MovingDown;
//				}
//				if (IsJumpPoint(r, c, -1, 0))
//				{
//					m_jumpPointMap[r][c] |= MovingUp;
//				}
//				if (IsJumpPoint(r, c, 0, 1))
//				{
//					m_jumpPointMap[r][c] |= MovingRight;
//				}
//				if (IsJumpPoint(r, c, 0, -1))
//				{
//					m_jumpPointMap[r][c] |= MovingLeft;
//				}
//			}
//		}
//	}
//}
//
//bool PrecomputeMap::IsJumpPoint(int r, int c, int rowDir, int colDir)
//{
//	return
//		IsEmpty(r - rowDir, c - colDir) &&						// Parent not a wall (not necessary)
//		((IsEmpty(r + colDir, c + rowDir) &&					// 1st forced neighbor
//			IsWall(r - rowDir + colDir, c - colDir + rowDir)) ||	// 1st forced neighbor (continued)
//			((IsEmpty(r - colDir, c - rowDir) &&					// 2nd forced neighbor
//				IsWall(r - rowDir - colDir, c - colDir - rowDir))));	// 2nd forced neighbor (continued)
//}

void PrecomputeMap::CalculateJumpPointMap()
{
	for (int r = 0; r < m_height; ++r)
	{
		for (int c = 0; c < m_width; ++c)
		{
			if (!m_map[c + (r * m_width)])
			{
				// North
				int north_r = r - 1, north_c = c;
				if (IsEmpty(north_r, north_c)) {
					// West and South-East is empty.
					if (IsEmpty(north_r, north_c - 1) && IsEmpty(north_r + 1, north_c + 1)) {
						m_jumpPointMap[north_r][north_c] |= IsJumpPoint;
						m_jumpPointMap[north_r][north_c] |= MovingRight;
						LOG("row %d, col %d, North %d.\n", r, c, 1);
					}
					// East and South-West is empty.
					if (IsEmpty(north_r, north_c + 1) && IsEmpty(north_r + 1, north_c - 1)) {
						m_jumpPointMap[north_r][north_c] |= IsJumpPoint;
						m_jumpPointMap[north_r][north_c] |= MovingLeft;
						LOG("row %d, col %d, North %d.\n", r, c, 2);
					}
					// South-East and South-West is empty.
					if (IsEmpty(north_r + 1, north_c + 1) && IsEmpty(north_r + 1, north_c - 1)) {
						m_jumpPointMap[north_r][north_c] |= IsJumpPoint;
						LOG("row %d, col %d, North %d.\n", r, c, 3);
					}
				}

				// East
				int east_r = r, east_c = c + 1;
				if (IsEmpty(east_r, east_c)) {
					// North and South-West is empty.
					if (IsEmpty(east_r - 1, east_c) && IsEmpty(east_r + 1, east_c - 1)) {
						m_jumpPointMap[east_r][east_c] |= IsJumpPoint;
						m_jumpPointMap[east_r][east_c] |= MovingDown;
						LOG("row %d, col %d, East %d.\n", r, c, 1);
					}
					// South and North-West is empty.
					if (IsEmpty(east_r + 1, east_c) && IsEmpty(east_r - 1, east_c - 1)) {
						m_jumpPointMap[east_r][east_c] |= IsJumpPoint;
						m_jumpPointMap[east_r][east_c] |= MovingUp;
						LOG("row %d, col %d, East %d.\n", r, c, 2);
					}
					// North-West and South-West is empty.
					if (IsEmpty(east_r - 1, east_c - 1) && IsEmpty(east_r + 1, east_c - 1)) {
						m_jumpPointMap[east_r][east_c] |= IsJumpPoint;
						LOG("row %d, col %d, East %d.\n", r, c, 3);
					}
				}

				// South
				int south_r = r + 1, south_c = c;
				if (IsEmpty(south_r, south_c)) {
					// West and North-East is empty.
					if (IsEmpty(south_r, south_c - 1) && IsEmpty(south_r - 1, south_c + 1)) {
						m_jumpPointMap[south_r][south_c] |= IsJumpPoint;
						m_jumpPointMap[south_r][south_c] |= MovingRight;
						LOG("row %d, col %d, South %d.\n", r, c, 1);
					}
					// East and North-West is empty.
					if (IsEmpty(south_r, south_c + 1) && IsEmpty(south_r - 1, south_c - 1)) {
						m_jumpPointMap[south_r][south_c] |= IsJumpPoint;
						m_jumpPointMap[south_r][south_c] |= MovingLeft;
						LOG("row %d, col %d, South %d.\n", r, c, 2);
					}
					// North-West and North-East is empty.
					if (IsEmpty(south_r - 1, south_c - 1) && IsEmpty(south_r - 1, south_c + 1)) {
						m_jumpPointMap[south_r][south_c] |= IsJumpPoint;
						LOG("row %d, col %d, South %d.\n", r, c, 3);
					}
				}

				// West
				int west_r = r, west_c = c - 1;
				if (IsEmpty(west_r, west_c)) {
					// North and South-East is empty.
					if (IsEmpty(west_r - 1, west_c) && IsEmpty(west_r + 1, west_c + 1)) {
						m_jumpPointMap[west_r][west_c] |= IsJumpPoint;
						m_jumpPointMap[west_r][west_c] |= MovingDown;
						LOG("row %d, col %d, West %d.\n", r, c, 1);
					}
					// South and North-East is empty.
					if (IsEmpty(west_r + 1, west_c) && IsEmpty(west_r - 1, west_c + 1)) {
						m_jumpPointMap[west_r][west_c] |= IsJumpPoint;
						m_jumpPointMap[west_r][west_c] |= MovingUp;
						LOG("row %d, col %d, West %d.\n", r, c, 2);
					}
					// South-East and North-East is empty.
					if (IsEmpty(west_r + 1, west_c + 1) && IsEmpty(west_r - 1, west_c + 1)) {
						m_jumpPointMap[west_r][west_c] |= IsJumpPoint;
						LOG("row %d, col %d, West %d.\n", r, c, 3);
					}
				}
			}
		}
	}

	//LOG("Map:\n");
	//for (int r = 0; r < m_height; ++r)
	//{
	//	for (int c = 0; c < m_width; ++c)
	//	{
	//		if (m_map[c + (r * m_width)]) {
	//			LOG("True  ");
	//		}
	//		else {
	//			LOG("False ");
	//		}
	//	}
	//	LOG("\n");
	//}

	for (int r = 0; r < m_height; ++r)
	{
		for (int c = 0; c < m_width; ++c)
		{
			if ((m_jumpPointMap[r][c] & IsJumpPoint) > 0)
			{
				LOG("Primary jump point: (%d, %d), num %d, direction: ", r, c, m_jumpPointMap[r][c]);
				if ((m_jumpPointMap[r][c] & MovingLeft) > 0) {
					LOG("Left ");
				}
				if ((m_jumpPointMap[r][c] & MovingRight) > 0) {
					LOG("Right ");
				}
				if ((m_jumpPointMap[r][c] & MovingUp) > 0) {
					LOG("Up ");
				}
				if ((m_jumpPointMap[r][c] & MovingDown) > 0) {
					LOG("Down ");
				}
				LOG("\n");
			}
		}
	}
}

inline bool PrecomputeMap::IsInBounds(int r, int c)
{
	unsigned int colBoundsCheck = c;
	unsigned int rowBoundsCheck = r;
	return (colBoundsCheck < (unsigned int)m_width &&
		rowBoundsCheck < (unsigned int)m_height);
}

inline bool PrecomputeMap::IsEmpty(int r, int c)
{
	if (IsInBounds(r, c))
	{
		return m_map[c + (r * m_width)];
	}
	else
	{
		return false;
	}
}

inline bool PrecomputeMap::IsWall(int r, int c)
{
	if (IsInBounds(r, c))
	{
		return !m_map[c + (r * m_width)];
	}
	else
	{
		return true;
	}
}

void PrecomputeMap::CalculateDistantJumpPointMap()
{
	// Calculate distant jump points (Left and Right)
	for (int r = 0; r < m_height; ++r)
	{
		{
			int countMovingLeft = -1;
			bool jumpPointLastSeen = false;
			for (int c = 0; c < m_width; ++c)
			{
				if (IsWall(r, c))
				{
					countMovingLeft = -1;
					jumpPointLastSeen = false;
					m_distantJumpPointMap[r][c].jumpDistance[Left] = 0;
					continue;
				}

				countMovingLeft++;

				if (jumpPointLastSeen)
				{
					m_distantJumpPointMap[r][c].jumpDistance[Left] = countMovingLeft;
				}
				else // Wall last seen
				{
					m_distantJumpPointMap[r][c].jumpDistance[Left] = -countMovingLeft;
				}

				if ((m_jumpPointMap[r][c] & MovingLeft) > 0)
				{
					countMovingLeft = 0;
					jumpPointLastSeen = true;
				}
			}
		}

		{
			int countMovingRight = -1;
			bool jumpPointLastSeen = false;
			for (int c = m_width - 1; c >= 0; --c)
			{
				if (IsWall(r, c))
				{
					countMovingRight = -1;
					jumpPointLastSeen = false;
					m_distantJumpPointMap[r][c].jumpDistance[Right] = 0;
					continue;
				}

				countMovingRight++;

				if (jumpPointLastSeen)
				{
					m_distantJumpPointMap[r][c].jumpDistance[Right] = countMovingRight;
				}
				else // Wall last seen
				{
					m_distantJumpPointMap[r][c].jumpDistance[Right] = -countMovingRight;
				}

				if ((m_jumpPointMap[r][c] & MovingRight) > 0)
				{
					countMovingRight = 0;
					jumpPointLastSeen = true;
				}
			}
		}
	}

	// Calculate distant jump points (Up and Down)
	for (int c = 0; c < m_width; ++c)
	{
		{
			int countMovingUp = -1;
			bool jumpPointLastSeen = false;
			for (int r = 0; r < m_height; ++r)
			{
				if (IsWall(r, c))
				{
					countMovingUp = -1;
					jumpPointLastSeen = false;
					m_distantJumpPointMap[r][c].jumpDistance[Up] = 0;
					continue;
				}

				countMovingUp++;

				if (jumpPointLastSeen)
				{
					m_distantJumpPointMap[r][c].jumpDistance[Up] = countMovingUp;
				}
				else // Wall last seen
				{
					m_distantJumpPointMap[r][c].jumpDistance[Up] = -countMovingUp;
				}

				if ((m_jumpPointMap[r][c] & MovingUp) > 0)
				{
					countMovingUp = 0;
					jumpPointLastSeen = true;
				}
			}
		}

		{
			int countMovingDown = -1;
			bool jumpPointLastSeen = false;
			for (int r = m_height - 1; r >= 0; --r)
			{
				if (IsWall(r, c))
				{
					countMovingDown = -1;
					jumpPointLastSeen = false;
					m_distantJumpPointMap[r][c].jumpDistance[Down] = 0;
					continue;
				}

				countMovingDown++;

				if (jumpPointLastSeen)
				{
					m_distantJumpPointMap[r][c].jumpDistance[Down] = countMovingDown;
				}
				else // Wall last seen
				{
					m_distantJumpPointMap[r][c].jumpDistance[Down] = -countMovingDown;
				}

				if ((m_jumpPointMap[r][c] & MovingDown) > 0)
				{
					countMovingDown = 0;
					jumpPointLastSeen = true;
				}
			}
		}
	}

	// Calculate distant jump points (Diagonally UpLeft and UpRight)
	for (int r = 0; r < m_height; ++r)
	{
		for (int c = 0; c < m_width; ++c)
		{
			if (IsEmpty(r, c))
			{
				if (r == 0 || c == 0 || (
					//IsWall(r - 1, c) || IsWall(r, c - 1) || 
					IsWall(r - 1, c - 1)))
				{
					// Wall one away
					m_distantJumpPointMap[r][c].jumpDistance[UpLeft] = 0;
				}
				else if (
					//IsEmpty(r - 1, c) && IsEmpty(r, c - 1) && 
					(m_distantJumpPointMap[r - 1][c - 1].jumpDistance[Up] > 0 ||
						m_distantJumpPointMap[r - 1][c - 1].jumpDistance[Left] > 0 ||
						(m_jumpPointMap[r - 1][c - 1] & IsJumpPoint) > 0))
				{
					// Diagonal one away
					m_distantJumpPointMap[r][c].jumpDistance[UpLeft] = 1;
				}
				else
				{
					// Increment from last
					int jumpDistance = m_distantJumpPointMap[r - 1][c - 1].jumpDistance[UpLeft];

					if (jumpDistance > 0)
					{
						m_distantJumpPointMap[r][c].jumpDistance[UpLeft] = 1 + jumpDistance;
					}
					else //if( jumpDistance <= 0 )
					{
						m_distantJumpPointMap[r][c].jumpDistance[UpLeft] = -1 + jumpDistance;
					}
				}

				if (r == 0 || c == m_width - 1 || (
					//IsWall(r - 1, c) || IsWall(r, c + 1) || 
					IsWall(r - 1, c + 1)))
				{
					// Wall one away
					m_distantJumpPointMap[r][c].jumpDistance[UpRight] = 0;
				}
				else if (
					//IsEmpty(r - 1, c) && IsEmpty(r, c + 1) &&
					(m_distantJumpPointMap[r - 1][c + 1].jumpDistance[Up] > 0 ||
						m_distantJumpPointMap[r - 1][c + 1].jumpDistance[Right] > 0 ||
						(m_jumpPointMap[r - 1][c + 1] & IsJumpPoint) > 0))
				{
					// Diagonal one away
					m_distantJumpPointMap[r][c].jumpDistance[UpRight] = 1;
				}
				else
				{
					// Increment from last
					int jumpDistance = m_distantJumpPointMap[r - 1][c + 1].jumpDistance[UpRight];

					if (jumpDistance > 0)
					{
						m_distantJumpPointMap[r][c].jumpDistance[UpRight] = 1 + jumpDistance;
					}
					else //if( jumpDistance <= 0 )
					{
						m_distantJumpPointMap[r][c].jumpDistance[UpRight] = -1 + jumpDistance;
					}
				}
			}
		}
	}

	// Calculate distant jump points (Diagonally DownLeft and DownRight)
	for (int r = m_height - 1; r >= 0; --r)
	{
		for (int c = 0; c < m_width; ++c)
		{
			if (IsEmpty(r, c))
			{
				if (r == m_height - 1 || c == 0 || (
					//IsWall(r + 1, c) || IsWall(r, c - 1) ||
					IsWall(r + 1, c - 1)))
				{
					// Wall one away
					m_distantJumpPointMap[r][c].jumpDistance[DownLeft] = 0;
				}
				else if (
					//IsEmpty(r + 1, c) && IsEmpty(r, c - 1) &&
					(m_distantJumpPointMap[r + 1][c - 1].jumpDistance[Down] > 0 ||
						m_distantJumpPointMap[r + 1][c - 1].jumpDistance[Left] > 0 ||
						(m_jumpPointMap[r + 1][c - 1] & IsJumpPoint) > 0))
				{
					// Diagonal one away
					m_distantJumpPointMap[r][c].jumpDistance[DownLeft] = 1;
				}
				else
				{
					// Increment from last
					int jumpDistance = m_distantJumpPointMap[r + 1][c - 1].jumpDistance[DownLeft];

					if (jumpDistance > 0)
					{
						m_distantJumpPointMap[r][c].jumpDistance[DownLeft] = 1 + jumpDistance;
					}
					else //if( jumpDistance <= 0 )
					{
						m_distantJumpPointMap[r][c].jumpDistance[DownLeft] = -1 + jumpDistance;
					}
				}

				if (r == m_height - 1 || c == m_width - 1 || (
					//IsWall(r + 1, c) || IsWall(r, c + 1) || 
					IsWall(r + 1, c + 1)))
				{
					// Wall one away
					m_distantJumpPointMap[r][c].jumpDistance[DownRight] = 0;
				}
				else if (
					//IsEmpty(r + 1, c) && IsEmpty(r, c + 1) &&
					(m_distantJumpPointMap[r + 1][c + 1].jumpDistance[Down] > 0 ||
						m_distantJumpPointMap[r + 1][c + 1].jumpDistance[Right] > 0 ||
						(m_jumpPointMap[r + 1][c + 1] & IsJumpPoint) > 0))
				{
					// Diagonal one away
					m_distantJumpPointMap[r][c].jumpDistance[DownRight] = 1;
				}
				else
				{
					// Increment from last
					int jumpDistance = m_distantJumpPointMap[r + 1][c + 1].jumpDistance[DownRight];

					if (jumpDistance > 0)
					{
						m_distantJumpPointMap[r][c].jumpDistance[DownRight] = 1 + jumpDistance;
					}
					else //if( jumpDistance <= 0 )
					{
						m_distantJumpPointMap[r][c].jumpDistance[DownRight] = -1 + jumpDistance;
					}
				}
			}
		}
	}
}
