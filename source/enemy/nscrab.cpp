/*=========================================================================

	nscrab.cpp

	Author:		CRB
	Created: 
	Project:	Spongebob
	Purpose: 

	Copyright (c) 2000 Climax Development Ltd

===========================================================================*/

#ifndef __ENEMY_NPC_H__
#include "enemy\npc.h"
#endif

#ifndef __ENEMY_NSCRAB_H__
#include "enemy\nscrab.h"
#endif

#ifndef __GAME_GAME_H__
#include	"game\game.h"
#endif

#ifndef	__PLAYER_PLAYER_H__
#include	"player\player.h"
#endif

#ifndef __ANIM_SPIDERCRAB_HEADER__
#include <ACTOR_SPIDERCRAB_ANIM.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcSpiderCrabEnemy::postInit()
{
	m_npcPath.setPathType( CNpcPath::PONG_PATH );

	if ( m_layerCollision->getHeightFromGround( Pos.vx, Pos.vy - 8, 16 ) <= 0 )
	{
		// starting off below ground, jump initially

		m_velocity = 5;

		m_state = SPIDER_CRAB_INIT_JUMP;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CNpcSpiderCrabEnemy::processSensor()
{
	switch( m_sensorFunc )
	{
		case NPC_SENSOR_NONE:
			return( false );

		default:
		{
			if ( abs( playerXDist ) < 64 )
			{
				// only attack if within path extents

				s32 minX, maxX;
				m_npcPath.getPathXExtents( &minX, &maxX );

				if ( playerXDist < 0 )
				{
					m_extendDir = EXTEND_LEFT;

					if ( ( Pos.vx + playerXDist ) < minX )
					{
						return( false );
					}
				}
				else
				{
					m_extendDir = EXTEND_RIGHT;

					if ( ( Pos.vx + playerXDist ) > maxX )
					{
						return( false );
					}
				}

				m_attackDist = abs( playerXDist );

				m_controlFunc = NPC_CONTROL_CLOSE;
				m_extension = 0;
				m_velocity = 5;
				m_base = Pos;

				return( true );
			}
			else
			{
				return( false );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcSpiderCrabEnemy::processClose( int _frames )
{
	s32 velocity;
	DVECTOR newPos = Pos;

	if ( m_animNo != ANIM_SPIDERCRAB_JUMP )
	{
		m_animPlaying = true;
		m_animNo = ANIM_SPIDERCRAB_JUMP;
		m_frame = 0;
	}

	velocity = m_velocity * _frames;

	if ( m_extendDir == EXTEND_RIGHT )
	{
		m_extension += velocity;
		m_heading = 0;
	}
	else
	{
		m_extension -= velocity;
		m_heading = 2048;
	}

	bool completed = false;

	if ( m_extension > m_attackDist )
	{
		m_extension = m_attackDist;
		completed = true;
	}
	else if ( m_extension < -m_attackDist )
	{
		m_extension = -m_attackDist;
		completed = true;
	}

	newPos.vx = m_base.vx + m_extension;
	newPos.vy = m_base.vy - ( ( SPIDER_CRAB_HEIGHT * rsin( abs( ( m_extension << 11 ) / m_attackDist ) ) ) >> 12 );

	s32 minX, maxX;

	m_npcPath.getPathXExtents( &minX, &maxX );

	if ( newPos.vx < minX )
	{
		newPos.vx = minX;
	}
	else if ( newPos.vx > maxX )
	{
		newPos.vx = maxX;
	}

	// check for collision with ground

	if ( m_layerCollision->getHeightFromGround( newPos.vx, newPos.vy ) < 0 )
	{
		// abort jump

		m_controlFunc = NPC_CONTROL_MOVEMENT;
		m_timerFunc = NPC_TIMER_ATTACK_DONE;
		m_timerTimer = GameState::getOneSecondInFrames();
		m_sensorFunc = NPC_SENSOR_NONE;

		m_extension = 0;
		completed = false;
	}
	else
	{
		Pos = newPos;
	}

	if ( completed )
	{
		m_controlFunc = NPC_CONTROL_MOVEMENT;
		m_timerFunc = NPC_TIMER_ATTACK_DONE;
		m_timerTimer = GameState::getOneSecondInFrames();
		m_sensorFunc = NPC_SENSOR_NONE;

		m_extension = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcSpiderCrabEnemy::processCollision()
{
	if ( m_oldControlFunc == NPC_CONTROL_CLOSE )
	{
		// bite player

		if ( m_animNo != ANIM_SPIDERCRAB_BITE )
		{
			CPlayer *player = GameScene.getPlayer();

			player->takeDamage( m_data[m_type].damageToUserType );

			m_animNo = ANIM_SPIDERCRAB_BITE;
			m_animPlaying = true;
			m_frame = 0;
		}
		else if ( !m_animPlaying )
		{
			m_controlFunc = NPC_CONTROL_MOVEMENT;
			m_timerFunc = NPC_TIMER_ATTACK_DONE;
			m_timerTimer = GameState::getOneSecondInFrames();
			m_sensorFunc = NPC_SENSOR_NONE;

			m_extension = 0;
		}
	}
	else
	{
		CPlayer *player = GameScene.getPlayer();

		player->takeDamage( m_data[m_type].damageToUserType );

		m_controlFunc = m_oldControlFunc;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcSpiderCrabEnemy::processSpiderCrabInitJumpMovement( int _frames )
{
	s32 velocity;
	bool completed = false;
	bool collision = false;

	if ( m_animNo != ANIM_SPIDERCRAB_JUMP )
	{
		m_animPlaying = true;
		m_animNo = ANIM_SPIDERCRAB_JUMP;
		m_frame = 0;
	}

	velocity = m_velocity * _frames;

	m_extension += velocity;

	if ( m_extension > 128 )
	{
		m_extension = 128;
		completed = true;
	}

	Pos.vy = m_base.vy - ( ( 50 * rsin( abs( m_extension ) << 4 ) ) >> 12 );

	/*if ( m_extension > 64 )
	{
		// check for collision on the way back down

		if ( m_layerCollision->getHeightFromGround( Pos.vx, Pos.vy ) < 0 )
		{
			collision = true;
		}
	}

	if ( completed || collision )
	{
		m_state = SPIDER_CRAB_DEFAULT;
	}*/

	if ( m_extension > 64 )
	{
		m_state = SPIDER_CRAB_DEFAULT;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcSpiderCrabEnemy::processMovement(int _frames)
{
	s32 moveX = 0, moveY = 0;
	s32 moveVel = 0;
	s32 moveDist = 0;

	if ( m_state == SPIDER_CRAB_INIT_JUMP )
	{
		processSpiderCrabInitJumpMovement( _frames );
	}
	else
	{
		processGenericFixedPathWalk( _frames, &moveX, &moveY );

		if ( !m_animPlaying )
		{
			m_animPlaying = true;
			m_animNo = m_data[m_type].moveAnim;
			m_frame = 0;
		}
	}

	processMovementModifier( _frames, moveX, moveY, moveVel, moveDist );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcSpiderCrabEnemy::processMovementModifier( int _frames, s32 distX, s32 distY, s32 dist, s16 headingChange )
{
	Pos.vx += distX;
	Pos.vy += distY;

	// sort out draw rotation

	DVECTOR testPos1, testPos2;

	testPos1 = testPos2 = Pos;
	testPos1.vx -= 10;
	testPos2.vx += 10;

	testPos1.vy += m_layerCollision->getHeightFromGround( testPos1.vx, testPos1.vy, 16 );
	testPos2.vy += m_layerCollision->getHeightFromGround( testPos2.vx, testPos2.vy, 16 );

	s32 xDist = testPos2.vx - testPos1.vx;
	s32 yDist = testPos2.vy - testPos1.vy;

	s16 heading = ratan2( yDist, xDist );

	m_drawRotation = heading;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcSpiderCrabEnemy::processEnemyCollision( CThing *thisThing )
{
	if ( m_state != SPIDER_CRAB_INIT_JUMP )
	{
		CNpcEnemy::processEnemyCollision( thisThing );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 CNpcSpiderCrabEnemy::canCollideWithEnemy()
{
	return( m_state != SPIDER_CRAB_INIT_JUMP );
}
