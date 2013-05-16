/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Daemon.

Daemon is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Daemon is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Daemon; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "g_bot_ai.h"
#include "g_bot_util.h"

void BotDPrintf( const char* fmt, ... )
{
	if ( g_bot_debug.integer )
	{
		va_list argptr;
		char    text[ 1024 ];

		va_start( argptr, fmt );
		Q_vsnprintf( text, sizeof( text ), fmt, argptr );
		va_end( argptr );

		trap_Print( text );
	}
}

void BotError( const char* fmt, ... )
{
	va_list argptr;
	size_t  len;
	char    text[ 1024 ] = S_COLOR_RED "ERROR: ";

	len = strlen( text );

	va_start( argptr, fmt );
	Q_vsnprintf( text + len, sizeof( text ) - len, fmt, argptr );
	va_end( argptr );

	trap_Print( text );
}

/*
=======================
Scoring functions for logic
=======================
*/
float BotGetBaseRushScore( gentity_t *ent )
{

	switch ( ent->s.weapon )
	{
		case WP_GRENADE:
			return 0.7f;
		case WP_BLASTER:
			return 0.1f;
		case WP_LUCIFER_CANNON:
			return 1.0f;
		case WP_MACHINEGUN:
			return 0.5f;
		case WP_PULSE_RIFLE:
			return 0.7f;
		case WP_LAS_GUN:
			return 0.7f;
		case WP_SHOTGUN:
			return 0.2f;
		case WP_CHAINGUN:
			if ( BG_InventoryContainsUpgrade( UP_BATTLESUIT, ent->client->ps.stats ) )
			{
				return 0.5f;
			}
			else
			{
				return 0.2f;
			}
		case WP_HBUILD:
			return 0.0f;
		case WP_ABUILD:
			return 0.0f;
		case WP_ABUILD2:
			return 0.0f;
		case WP_ALEVEL0:
			return 0.0f;
		case WP_ALEVEL1:
			return 0.2f;
		case WP_ALEVEL2:
			return 0.5f;
		case WP_ALEVEL2_UPG:
			return 0.7f;
		case WP_ALEVEL3:
			return 0.8f;
		case WP_ALEVEL3_UPG:
			return 0.9f;
		case WP_ALEVEL4:
			return 1.0f;
		default:
			return 0.5f;
	}
}

float BotGetHealScore( gentity_t *self )
{
	float distToHealer = 0;
	float percentHealth = 0;
	float maxHealth = BG_Class( ( class_t ) self->client->ps.stats[ STAT_CLASS ] )->health;

	if ( self->client->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
	{
		if ( self->botMind->closestBuildings[ BA_A_BOOSTER ].ent )
		{
			distToHealer = self->botMind->closestBuildings[ BA_A_BOOSTER ].distance;
		}
		else if ( self->botMind->closestBuildings[ BA_A_OVERMIND ].ent )
		{
			distToHealer = self->botMind->closestBuildings[ BA_A_OVERMIND ].distance;
		}
		else if ( self->botMind->closestBuildings[ BA_A_SPAWN ].ent )
		{
			distToHealer = self->botMind->closestBuildings[BA_A_SPAWN].distance;
		}
	}
	else
	{
		distToHealer = self->botMind->closestBuildings[ BA_H_MEDISTAT ].distance;
	}

	percentHealth = ( ( float ) self->client->ps.stats[STAT_HEALTH] ) / maxHealth;

	distToHealer = MAX( MIN( MAX_HEAL_DIST, distToHealer ), MAX_HEAL_DIST * ( 3.0f / 4.0f ) );

	if ( percentHealth == 1.0f )
	{
		return 1.0f;
	}
	return percentHealth * distToHealer / MAX_HEAL_DIST;
}

float BotGetEnemyPriority( gentity_t *self, gentity_t *ent )
{
	float enemyScore;
	float distanceScore;
	distanceScore = Distance( self->s.origin, ent->s.origin );

	if ( ent->client )
	{
		switch ( ent->s.weapon )
		{
			case WP_ALEVEL0:
				enemyScore = 0.1;
				break;
			case WP_ALEVEL1:
			case WP_ALEVEL1_UPG:
				enemyScore = 0.3;
				break;
			case WP_ALEVEL2:
				enemyScore = 0.4;
				break;
			case WP_ALEVEL2_UPG:
				enemyScore = 0.7;
				break;
			case WP_ALEVEL3:
				enemyScore = 0.7;
				break;
			case WP_ALEVEL3_UPG:
				enemyScore = 0.8;
				break;
			case WP_ALEVEL4:
				enemyScore = 1.0;
				break;
			case WP_BLASTER:
				enemyScore = 0.2;
				break;
			case WP_MACHINEGUN:
				enemyScore = 0.4;
				break;
			case WP_PAIN_SAW:
				enemyScore = 0.4;
				break;
			case WP_LAS_GUN:
				enemyScore = 0.4;
				break;
			case WP_MASS_DRIVER:
				enemyScore = 0.4;
				break;
			case WP_CHAINGUN:
				enemyScore = 0.6;
				break;
			case WP_FLAMER:
				enemyScore = 0.6;
				break;
			case WP_PULSE_RIFLE:
				enemyScore = 0.5;
				break;
			case WP_LUCIFER_CANNON:
				enemyScore = 1.0;
				break;
			default:
				enemyScore = 0.5;
				break;
		}
	}
	else
	{
		switch ( ent->s.modelindex )
		{
			case BA_H_MGTURRET:
				enemyScore = 0.6;
				break;
			case BA_H_REACTOR:
				enemyScore = 0.5;
				break;
			case BA_H_TESLAGEN:
				enemyScore = 0.7;
				break;
			case BA_H_SPAWN:
				enemyScore = 0.9;
				break;
			case BA_H_ARMOURY:
				enemyScore = 0.8;
				break;
			case BA_H_MEDISTAT:
				enemyScore = 0.6;
				break;
			case BA_H_DCC:
				enemyScore = 0.5;
				break;
			case BA_A_ACIDTUBE:
				enemyScore = 0.7;
				break;
			case BA_A_SPAWN:
				enemyScore = 0.9;
				break;
			case BA_A_OVERMIND:
				enemyScore = 0.5;
				break;
			case BA_A_HIVE:
				enemyScore = 0.7;
				break;
			case BA_A_BOOSTER:
				enemyScore = 0.4;
				break;
			case BA_A_TRAPPER:
				enemyScore = 0.8;
				break;
			default:
				enemyScore = 0.5;
				break;

		}
	}
	return enemyScore * 1000 / distanceScore;
}


qboolean BotCanEvolveToClass( gentity_t *self, class_t newClass )
{
	return ( BG_ClassCanEvolveFromTo( ( class_t )self->client->ps.stats[STAT_CLASS], newClass, self->client->ps.persistant[PERS_CREDIT], g_alienStage.integer, 0 ) >= 0 );
}

qboolean WeaponIsEmpty( weapon_t weapon, playerState_t ps )
{
	if ( ps.ammo <= 0 && ps.clips <= 0 && !BG_Weapon( weapon )->infiniteAmmo )
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

float PercentAmmoRemaining( weapon_t weapon, playerState_t *ps )
{
	int maxAmmo, maxClips;
	float totalMaxAmmo, totalAmmo;

	maxAmmo = BG_Weapon( weapon )->maxAmmo;
	maxClips = BG_Weapon( weapon )->maxClips;
	if ( !BG_Weapon( weapon )->infiniteAmmo )
	{
		if ( BG_InventoryContainsUpgrade( UP_BATTPACK, ps->stats ) )
		{
			maxAmmo = ( int )( ( float )maxAmmo * BATTPACK_MODIFIER );
		}

		totalMaxAmmo = ( float ) maxAmmo + maxClips * maxAmmo;
		totalAmmo = ( float ) ps->ammo + ps->clips * maxAmmo;

		return ( float ) totalAmmo / totalMaxAmmo;
	}
	else
	{
		return 1.0f;
	}
}

int BotValueOfWeapons( gentity_t *self )
{
	int worth = 0;
	int i;

	for ( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
	{
		if ( BG_InventoryContainsWeapon( i, self->client->ps.stats ) )
		{
			worth += BG_Weapon( ( weapon_t )i )->price;
		}
	}
	return worth;
}
int BotValueOfUpgrades( gentity_t *self )
{
	int worth = 0;
	int i;

	for ( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
	{
		if ( BG_InventoryContainsUpgrade( i, self->client->ps.stats ) )
		{
			worth += BG_Upgrade( ( upgrade_t ) i )->price;
		}
	}
	return worth;
}

void BotGetDesiredBuy( gentity_t *self, weapon_t *weapon, upgrade_t *upgrades, int *numUpgrades )
{
	int i;
	int equipmentPrice = BotValueOfWeapons( self ) + BotValueOfUpgrades( self );
	int credits = self->client->ps.persistant[PERS_CREDIT];
	int usableCapital = credits + equipmentPrice;

	//decide what upgrade(s) to buy
	if ( g_humanStage.integer >= 2 && usableCapital >= ( BG_Weapon( WP_PAIN_SAW )->price + BG_Upgrade( UP_BATTLESUIT )->price ) )
	{
		upgrades[0] = UP_BATTLESUIT;
		*numUpgrades = 1;
	}
	else if ( g_humanStage.integer >= 1 && usableCapital >= ( BG_Weapon( WP_SHOTGUN )->price + BG_Upgrade( UP_LIGHTARMOUR )->price + BG_Upgrade( UP_HELMET )->price ) )
	{
		upgrades[0] = UP_LIGHTARMOUR;
		upgrades[1] = UP_HELMET;
		*numUpgrades = 2;
	}
	else if ( g_humanStage.integer >= 0 && usableCapital >= ( BG_Weapon( WP_PAIN_SAW )->price + BG_Upgrade( UP_LIGHTARMOUR )->price ) )
	{
		upgrades[0] = UP_LIGHTARMOUR;
		*numUpgrades = 1;
	}
	else
	{
		*numUpgrades = 0;
	}

	for (i = 0; i < *numUpgrades; i++)
	{
		usableCapital -= BG_Upgrade( upgrades[i] )->price;
	}

	//now decide what weapon to buy
	if ( g_humanStage.integer >= 2  && usableCapital >= BG_Weapon( WP_LUCIFER_CANNON )->price && g_bot_lcannon.integer )
	{
		*weapon = WP_LUCIFER_CANNON;;
	}
	else if ( g_humanStage.integer >= 2 && usableCapital >= BG_Weapon( WP_CHAINGUN )->price && upgrades[0] == UP_BATTLESUIT && g_bot_chaingun.integer )
	{
		*weapon = WP_CHAINGUN;
	}
	else if ( g_humanStage.integer >= 1 && g_alienStage.integer < 2 && usableCapital >= BG_Weapon( WP_FLAMER )->price && g_bot_flamer.integer )
	{
		*weapon = WP_FLAMER;
	}
	else if ( g_humanStage.integer >= 1 && usableCapital >= BG_Weapon( WP_PULSE_RIFLE )->price && g_bot_prifle.integer )
	{
		*weapon = WP_PULSE_RIFLE;
	}
	else if ( g_humanStage.integer >= 0 && usableCapital >= BG_Weapon( WP_CHAINGUN )->price && g_bot_chaingun.integer )
	{
		*weapon = WP_CHAINGUN;;
	}
	else if ( g_humanStage.integer >= 0 && usableCapital >= BG_Weapon( WP_MASS_DRIVER )->price && g_bot_mdriver.integer )
	{
		*weapon = WP_MASS_DRIVER;
	}
	else if ( g_humanStage.integer >= 0 && usableCapital >= BG_Weapon( WP_LAS_GUN )->price && g_bot_lasgun.integer )
	{
		*weapon = WP_LAS_GUN;
	}
	else if ( g_humanStage.integer >= 0 && usableCapital >= BG_Weapon( WP_SHOTGUN )->price && g_bot_shotgun.integer )
	{
		*weapon = WP_SHOTGUN;
	}
	else if ( g_humanStage.integer >= 0 && usableCapital >= BG_Weapon( WP_PAIN_SAW )->price && g_bot_painsaw.integer )
	{
		*weapon = WP_PAIN_SAW;
	}
	else
	{
		*weapon = WP_MACHINEGUN;
	}

	usableCapital -= BG_Weapon( *weapon )->price;

	//finally, see if we can buy a battpack
	if ( BG_Weapon( *weapon )->usesEnergy && usableCapital >= BATTPACK_PRICE && g_humanStage.integer >= 1 && upgrades[0] != UP_BATTLESUIT )
	{
		upgrades[( *numUpgrades )++] = UP_BATTPACK;
		usableCapital -= BATTPACK_PRICE;
	}

	//now test to see if we already have all of these items
	//check if we already have everything
	if ( BG_InventoryContainsWeapon( ( int )*weapon, self->client->ps.stats ) )
	{
		int numContain = 0;
		int i;

		for ( i = 0; i < *numUpgrades; i++ )
		{
			if ( BG_InventoryContainsUpgrade( ( int )upgrades[i], self->client->ps.stats ) )
			{
				numContain++;
			}
		}
		//we have every upgrade we want to buy, and the weapon we want to buy, so test if we need ammo
		if ( numContain == *numUpgrades )
		{
			*numUpgrades = 0;
			for ( i = 0; i < 3; i++ )
			{
				upgrades[i] = UP_NONE;
			}
			if ( PercentAmmoRemaining( BG_PrimaryWeapon( self->client->ps.stats ), &self->client->ps ) < BOT_LOW_AMMO )
			{
				upgrades[0] = UP_AMMO;
				*numUpgrades = 1;
			}
			*weapon = WP_NONE;
		}
	}
}
/*
=======================
Entity Querys
=======================
*/
gentity_t* BotFindBuilding( gentity_t *self, int buildingType, int range )
{
	float minDistance = -1;
	gentity_t* closestBuilding = NULL;
	float newDistance;
	float rangeSquared = Square( range );
	gentity_t *target = &g_entities[MAX_CLIENTS];
	int i;

	for ( i = MAX_CLIENTS; i < level.num_entities; i++, target++ )
	{
		if ( !target->inuse )
		{
			continue;
		}
		if ( target->s.eType == ET_BUILDABLE && target->s.modelindex == buildingType && ( target->buildableTeam == TEAM_ALIENS || ( target->powered && target->spawned ) ) && target->health > 0 )
		{
			newDistance = DistanceSquared( self->s.origin, target->s.origin );
			if ( range && newDistance > rangeSquared )
			{
				continue;
			}
			if ( newDistance < minDistance || minDistance == -1 )
			{
				minDistance = newDistance;
				closestBuilding = target;
			}
		}
	}
	return closestBuilding;
}

void BotFindClosestBuildings( gentity_t *self, botEntityAndDistance_t *closest )
{
	gentity_t *testEnt;
	botEntityAndDistance_t *ent;
	memset( closest, 0, sizeof( botEntityAndDistance_t ) * BA_NUM_BUILDABLES );

	for ( testEnt = &g_entities[MAX_CLIENTS]; testEnt < &g_entities[level.num_entities - 1]; testEnt++ )
	{
		float newDist;
		//ignore entities that arnt in use
		if ( !testEnt->inuse )
		{
			continue;
		}

		//ignore dead targets
		if ( testEnt->health <= 0 )
		{
			continue;
		}

		//skip non buildings
		if ( testEnt->s.eType != ET_BUILDABLE )
		{
			continue;
		}

		//skip human buildings that are currently building or arn't powered
		if ( testEnt->buildableTeam == TEAM_HUMANS && ( !testEnt->powered || !testEnt->spawned ) )
		{
			continue;
		}

		newDist = Distance( self->s.origin, testEnt->s.origin );

		ent = &closest[ testEnt->s.modelindex ];
		if ( newDist < ent->distance || ent->distance == 0 )
		{
			ent->ent = testEnt;
			ent->distance = newDist;
		}
	}
}

gentity_t* BotFindDamagedFriendlyStructure( gentity_t *self )
{
	//closest building
	gentity_t* closestBuilding = NULL;

	//minimum distance found
	float minDistance = Square( ALIENSENSE_RANGE );

	gentity_t *target;

	for ( target = &g_entities[MAX_CLIENTS]; target < &g_entities[level.num_entities - 1]; target++ )
	{
		float distance;

		if ( target->s.eType != ET_BUILDABLE )
		{
			continue;
		}
		if ( target->buildableTeam != TEAM_HUMANS )
		{
			continue;
		}
		if ( target->health >= BG_Buildable( ( buildable_t )target->s.modelindex )->health )
		{
			continue;
		}
		if ( target->health <= 0 )
		{
			continue;
		}
		if ( !target->spawned || !target->powered )
		{
			continue;
		}

		distance = DistanceSquared( self->s.origin, target->s.origin );
		if ( distance <= minDistance )
		{
			minDistance = distance;
			closestBuilding = target;
		}
	}
	return closestBuilding;
}

qboolean BotEntityIsVisible( gentity_t *self, gentity_t *target, int mask )
{
	botTarget_t bt;
	BotSetTarget( &bt, target, NULL );
	return BotTargetIsVisible( self, bt, mask );
}

gentity_t* BotFindBestEnemy( gentity_t *self )
{
	float bestVisibleEnemyScore = 0;
	float bestInvisibleEnemyScore = 0;
	gentity_t *bestVisibleEnemy = NULL;
	gentity_t *bestInvisibleEnemy = NULL;
	gentity_t *target;

	for ( target = g_entities; target < &g_entities[level.num_entities - 1]; target++ )
	{
		float newScore;
		//ignore entities that arnt in use
		if ( !target->inuse )
		{
			continue;
		}

		//ignore dead targets
		if ( target->health <= 0 )
		{
			continue;
		}

		//ignore buildings if we cant attack them
		if ( target->s.eType == ET_BUILDABLE && ( !g_bot_attackStruct.integer || self->client->ps.stats[STAT_CLASS] == PCL_ALIEN_LEVEL0 ) )
		{
			continue;
		}

		//ignore neutrals
		if ( BotGetEntityTeam( target ) == TEAM_NONE )
		{
			continue;
		}

		//ignore teamates
		if ( BotGetEntityTeam( target ) == BotGetEntityTeam( self ) )
		{
			continue;
		}

		//ignore spectators
		if ( target->client )
		{
			if ( target->client->sess.spectatorState != SPECTATOR_NOT )
			{
				continue;
			}
		}

		if ( DistanceSquared( self->s.origin, target->s.origin ) > Square( ALIENSENSE_RANGE ) )
		{
			continue;
		}

		newScore = BotGetEnemyPriority( self, target );

		if ( newScore > bestVisibleEnemyScore && BotEntityIsVisible( self, target, MASK_SHOT ) )
		{
			//store the new score and the index of the entity
			bestVisibleEnemyScore = newScore;
			bestVisibleEnemy = target;
		}
		else if ( newScore > bestInvisibleEnemyScore && BotGetEntityTeam( self ) == TEAM_ALIENS )
		{
			bestInvisibleEnemyScore = newScore;
			bestInvisibleEnemy = target;
		}
	}
	if ( bestVisibleEnemy || BotGetEntityTeam( self ) == TEAM_HUMANS )
	{
		return bestVisibleEnemy;
	}
	else
	{
		return bestInvisibleEnemy;
	}
}

gentity_t* BotFindClosestEnemy( gentity_t *self )
{
	gentity_t* closestEnemy = NULL;
	float minDistance = Square( ALIENSENSE_RANGE );
	gentity_t *target;

	for ( target = g_entities; target < &g_entities[level.num_entities - 1]; target++ )
	{
		float newDistance;
		//ignore entities that arnt in use
		if ( !target->inuse )
		{
			continue;
		}

		//ignore dead targets
		if ( target->health <= 0 )
		{
			continue;
		}

		//ignore buildings if we cant attack them
		if ( target->s.eType == ET_BUILDABLE && ( !g_bot_attackStruct.integer || self->client->ps.stats[STAT_CLASS] == PCL_ALIEN_LEVEL0 ) )
		{
			continue;
		}

		//ignore neutrals
		if ( BotGetEntityTeam( target ) == TEAM_NONE )
		{
			continue;
		}

		//ignore teamates
		if ( BotGetEntityTeam( target ) == BotGetEntityTeam( self ) )
		{
			continue;
		}

		//ignore spectators
		if ( target->client )
		{
			if ( target->client->sess.spectatorState != SPECTATOR_NOT )
			{
				continue;
			}
		}
		newDistance = DistanceSquared( self->s.origin, target->s.origin );
		if ( newDistance <= minDistance )
		{
			minDistance = newDistance;
			closestEnemy = target;
		}
	}
	return closestEnemy;
}

botTarget_t BotGetRushTarget( gentity_t *self )
{
	botTarget_t target;
	gentity_t* rushTarget = NULL;
	if ( BotGetEntityTeam( self ) == TEAM_HUMANS )
	{
		if ( self->botMind->closestBuildings[BA_A_SPAWN].ent )
		{
			rushTarget = self->botMind->closestBuildings[BA_A_SPAWN].ent;
		}
		else if ( self->botMind->closestBuildings[BA_A_OVERMIND].ent )
		{
			rushTarget = self->botMind->closestBuildings[BA_A_OVERMIND].ent;
		}
	}
	else    //team aliens
	{
		if ( self->botMind->closestBuildings[BA_H_SPAWN].ent )
		{
			rushTarget = self->botMind->closestBuildings[BA_H_SPAWN].ent;
		}
		else if ( self->botMind->closestBuildings[BA_H_REACTOR].ent )
		{
			rushTarget = self->botMind->closestBuildings[BA_H_REACTOR].ent;
		}
	}
	BotSetTarget( &target, rushTarget, NULL );
	return target;
}

botTarget_t BotGetRetreatTarget( gentity_t *self )
{
	botTarget_t target;
	gentity_t* retreatTarget = NULL;
	//FIXME, this seems like it could be done better...
	if ( self->client->ps.stats[STAT_TEAM] == TEAM_HUMANS )
	{
		if ( self->botMind->closestBuildings[BA_H_REACTOR].ent )
		{
			retreatTarget = self->botMind->closestBuildings[BA_H_REACTOR].ent;
		}
	}
	else
	{
		if ( self->botMind->closestBuildings[BA_A_OVERMIND].ent )
		{
			retreatTarget = self->botMind->closestBuildings[BA_A_OVERMIND].ent;
		}
	}
	BotSetTarget( &target, retreatTarget, NULL );
	return target;
}

botTarget_t BotGetRoamTarget( gentity_t *self )
{
	botTarget_t target;
	vec3_t targetPos;

	BotFindRandomPointOnMesh( self, targetPos );
	BotSetTarget( &target, NULL, targetPos );
	return target;
}
/*
========================
BotTarget Helpers
========================
*/

void BotSetTarget( botTarget_t *target, gentity_t *ent, vec3_t pos )
{
	if ( ent )
	{
		target->ent = ent;
		VectorClear( target->coord );
		target->inuse = qtrue;
	}
	else if ( pos )
	{
		target->ent = NULL;
		VectorCopy( pos, target->coord );
		target->inuse = qtrue;
	}
	else
	{
		target->ent = NULL;
		VectorClear( target->coord );
		target->inuse = qfalse;
	}
}

qboolean BotTargetIsEntity( botTarget_t target )
{
	return ( target.ent && target.ent->inuse );
}

qboolean BotTargetIsPlayer( botTarget_t target )
{
	return ( target.ent && target.ent->inuse && target.ent->client );
}

int BotGetTargetEntityNumber( botTarget_t target )
{
	if ( BotTargetIsEntity( target ) )
	{
		return target.ent->s.number;
	}
	else
	{
		return ENTITYNUM_NONE;
	}
}

void BotGetTargetPos( botTarget_t target, vec3_t rVec )
{
	if ( BotTargetIsEntity( target ) )
	{
		VectorCopy( target.ent->s.origin, rVec );
	}
	else
	{
		VectorCopy( target.coord, rVec );
	}
}

void BotTargetToRouteTarget( gentity_t *self, botTarget_t target, botRouteTarget_t *routeTarget )
{
	vec3_t mins, maxs;
	int i;

	if ( BotTargetIsEntity( target ) )
	{
		VectorCopy( target.ent->r.mins, mins );
		VectorCopy( target.ent->r.maxs, maxs );

		if ( BotTargetIsPlayer( target ) )
		{
			routeTarget->type = BOT_TARGET_DYNAMIC;
		}
		else
		{
			routeTarget->type = BOT_TARGET_STATIC;
		}
	}
	else
	{
		// point target
		VectorSet( maxs, 96, 96, 96 );
		VectorSet( mins, -96, -96, -96 );
		routeTarget->type = BOT_TARGET_STATIC;
	}

	for ( i = 0; i < 3; i++ )
	{
		mins[ i ] = Q_fabs( mins[ i ] );
	}
	
	for ( i = 0; i < 3; i++ )
	{
		routeTarget->polyExtents[ i ] = MAX( mins[ i ], maxs[ i ] );
	}

	BotGetTargetPos( target, routeTarget->pos );

	// move center a bit lower so we don't get polys above the object 
	// and get polys below the object on a slope
	routeTarget->pos[ 2 ] -= routeTarget->polyExtents[ 2 ] / 2;

	// increase extents a little to account for obstacles cutting into the navmesh
	// also accounts for navmesh erosion at mesh boundrys
	routeTarget->polyExtents[ 0 ] += self->r.maxs[ 0 ] + 10;
	routeTarget->polyExtents[ 1 ] += self->r.maxs[ 1 ] + 10;
}

team_t BotGetEntityTeam( gentity_t *ent )
{
	if ( !ent )
	{
		return TEAM_NONE;
	}
	if ( ent->client )
	{
		return ( team_t )ent->client->ps.stats[STAT_TEAM];
	}
	else if ( ent->s.eType == ET_BUILDABLE )
	{
		return ent->buildableTeam;
	}
	else
	{
		return TEAM_NONE;
	}
}

team_t BotGetTargetTeam( botTarget_t target )
{
	if ( BotTargetIsEntity( target ) )
	{
		return BotGetEntityTeam( target.ent );
	}
	else
	{
		return TEAM_NONE;
	}
}

int BotGetTargetType( botTarget_t target )
{
	if ( BotTargetIsEntity( target ) )
	{
		return target.ent->s.eType;
	}
	else
	{
		return -1;
	}
}

qboolean BotChangeGoal( gentity_t *self, botTarget_t target )
{
	if ( !target.inuse )
	{
		return qfalse;
	}

	if ( FindRouteToTarget( self, target ) & ( ROUTE_PARTIAL | ROUTE_FAILED ) )
	{
		return qfalse;
	}

	self->botMind->goal = target;
	self->botMind->directPathToGoal = qfalse;
	return qtrue;
}

qboolean BotChangeGoalEntity( gentity_t *self, gentity_t *goal )
{
	botTarget_t target;
	BotSetTarget( &target, goal, NULL );
	return BotChangeGoal( self, target );
}

qboolean BotChangeGoalPos( gentity_t *self, vec3_t goal )
{
	botTarget_t target;
	BotSetTarget( &target, NULL, goal );
	return BotChangeGoal( self, target );
}

qboolean BotTargetInAttackRange( gentity_t *self, botTarget_t target )
{
	float range, secondaryRange;
	vec3_t forward, right, up;
	vec3_t muzzle;
	vec3_t maxs, mins;
	vec3_t targetPos;
	trace_t trace;
	float width = 0, height = 0;

	AngleVectors( self->client->ps.viewangles, forward, right, up );
	CalcMuzzlePoint( self, forward, right, up , muzzle );
	BotGetTargetPos( target, targetPos );
	switch ( self->client->ps.weapon )
	{
		case WP_ABUILD:
			range = ABUILDER_CLAW_RANGE;
			secondaryRange = 0;
			width = height = ABUILDER_CLAW_WIDTH;
			break;
		case WP_ABUILD2:
			range = ABUILDER_CLAW_RANGE;
			secondaryRange = 300; //An arbitrary value for the blob launcher, has nothing to do with actual range
			width = height = ABUILDER_CLAW_WIDTH;
			break;
		case WP_ALEVEL0:
			range = LEVEL0_BITE_RANGE;
			secondaryRange = 0;
			break;
		case WP_ALEVEL1:
			range = LEVEL1_CLAW_RANGE;
			secondaryRange = 0;
			width = height = LEVEL1_CLAW_WIDTH;
			break;
		case WP_ALEVEL1_UPG:
			range = LEVEL1_CLAW_RANGE;
			secondaryRange = LEVEL1_PCLOUD_RANGE;
			width = height = LEVEL1_CLAW_WIDTH;
			break;
		case WP_ALEVEL2:
			range = LEVEL2_CLAW_RANGE;
			secondaryRange = 0;
			width = height = LEVEL2_CLAW_WIDTH;
			break;
		case WP_ALEVEL2_UPG:
			range = LEVEL2_CLAW_U_RANGE;
			secondaryRange = LEVEL2_AREAZAP_RANGE;
			width = height = LEVEL2_CLAW_WIDTH;
			break;
		case WP_ALEVEL3:
			range = LEVEL3_CLAW_RANGE;
			//need to check if we can pounce to the target
			secondaryRange = LEVEL3_POUNCE_JUMP_MAG; //An arbitrary value for pounce, has nothing to do with actual range
			width = height = LEVEL3_CLAW_WIDTH;
			break;
		case WP_ALEVEL3_UPG:
			range = LEVEL3_CLAW_RANGE;
			//we can pounce, or we have barbs
			secondaryRange = LEVEL3_POUNCE_JUMP_MAG_UPG; //An arbitrary value for pounce and barbs, has nothing to do with actual range
			if ( self->client->ps.ammo > 0 )
			{
				secondaryRange = 900;
			}
			width = height = LEVEL3_CLAW_WIDTH;
			break;
		case WP_ALEVEL4:
			range = LEVEL4_CLAW_RANGE;
			secondaryRange = 0; //Using 0 since tyrant rush is basically just movement, not a ranged attack
			width = height = LEVEL4_CLAW_WIDTH;
			break;
		case WP_HBUILD:
			range = 100;
			secondaryRange = 0;
			break;
		case WP_PAIN_SAW:
			range = PAINSAW_RANGE;
			secondaryRange = 0;
			break;
		case WP_FLAMER:
			{
				vec3_t dir;
				vec3_t rdir;
				vec3_t nvel;
				vec3_t npos;
				vec3_t proj;
				trajectory_t t;
			
				// Correct muzzle so that the missile does not start in the ceiling
				VectorMA( muzzle, -7.0f, up, muzzle );

				// Correct muzzle so that the missile fires from the player's hand
				VectorMA( muzzle, 4.5f, right, muzzle );

				// flamer projectiles add the player's velocity scaled by FLAMER_LAG to the fire direction with length FLAMER_SPEED
				VectorSubtract( targetPos, muzzle, dir );
				VectorNormalize( dir );
				VectorScale( self->client->ps.velocity, FLAMER_LAG, nvel );
				VectorMA( nvel, FLAMER_SPEED, dir, t.trDelta );
				SnapVector( t.trDelta );
				VectorCopy( muzzle, t.trBase );
				t.trType = TR_LINEAR;
				t.trTime = level.time - 50;
			
				// find projectile's final position
				BG_EvaluateTrajectory( &t, level.time + FLAMER_LIFETIME, npos );

				// find distance traveled by projectile along fire line
				ProjectPointOntoVector( npos, muzzle, targetPos, proj );
				range = Distance( muzzle, proj );

				// make sure the sign of the range is correct
				VectorSubtract( npos, muzzle, rdir );
				VectorNormalize( rdir );
				if ( DotProduct( rdir, dir ) < 0 )
				{
					range = -range;
				}

				// decrease range to prevent problems from the approximation
				range -= 100;
				secondaryRange = 0;
				width = height = FLAMER_SIZE;
			}
			break;
		case WP_SHOTGUN:
			range = ( 50 * 8192 ) / SHOTGUN_SPREAD; //50 is the maximum radius we want the spread to be
			secondaryRange = 0;
			break;
		case WP_MACHINEGUN:
			range = ( 100 * 8192 ) / RIFLE_SPREAD; //100 is the maximum radius we want the spread to be
			secondaryRange = 0;
			break;
		case WP_CHAINGUN:
			range = ( 60 * 8192 ) / CHAINGUN_SPREAD; //60 is the maximum radius we want the spread to be
			secondaryRange = 0;
			break;
		default:
			range = 4098 * 4; //large range for guns because guns have large ranges :)
			secondaryRange = 0; //no secondary attack
	}
	VectorSet( maxs, width, width, width );
	VectorSet( mins, -width, -width, -height );

	trap_Trace( &trace, muzzle, mins, maxs, targetPos, self->s.number, MASK_SHOT );

	if ( self->client->ps.stats[STAT_TEAM] != BotGetEntityTeam( &g_entities[trace.entityNum] ) 
		&& BotGetEntityTeam( &g_entities[ trace.entityNum ] ) != TEAM_NONE
		&& Distance( muzzle, trace.endpos ) <= MAX( range, secondaryRange ) )
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

qboolean BotTargetIsVisible( gentity_t *self, botTarget_t target, int mask )
{
	trace_t trace;
	vec3_t  muzzle, targetPos;
	vec3_t  forward, right, up;

	AngleVectors( self->client->ps.viewangles, forward, right, up );
	CalcMuzzlePoint( self, forward, right, up, muzzle );
	BotGetTargetPos( target, targetPos );

	if ( !trap_InPVS( muzzle, targetPos ) )
	{
		return qfalse;
	}

	if ( mask == CONTENTS_SOLID )
	{
		trap_TraceNoEnts( &trace, muzzle, NULL, NULL, targetPos, self->s.number, mask );
	}
	else
	{
		trap_Trace( &trace, muzzle, NULL, NULL, targetPos, self->s.number, mask );
	}

	if ( trace.surfaceFlags & SURF_NOIMPACT )
	{
		return qfalse;
	}

	//target is in range
	if ( ( trace.entityNum == BotGetTargetEntityNumber( target ) || trace.fraction == 1.0f ) && !trace.startsolid )
	{
		return qtrue;
	}
	return qfalse;
}
/*
========================
Bot Aiming
========================
*/
void BotGetIdealAimLocation( gentity_t *self, botTarget_t target, vec3_t aimLocation )
{
	//get the position of the target
	BotGetTargetPos( target, aimLocation );

	if ( BotGetTargetType( target ) != ET_BUILDABLE && BotTargetIsEntity( target ) && BotGetTargetTeam( target ) == TEAM_HUMANS )
	{

		//aim at head
		aimLocation[2] += target.ent->r.maxs[2] * 0.85;

	}
	else if ( BotGetTargetType( target ) == ET_BUILDABLE || BotGetTargetTeam( target ) == TEAM_ALIENS )
	{
		//make lucifer cannons aim ahead based on the target's velocity
		if ( self->client->ps.weapon == WP_LUCIFER_CANNON && self->botMind->botSkill.level >= 5 )
		{
			VectorMA( aimLocation, Distance( self->s.origin, aimLocation ) / LCANNON_SPEED, target.ent->s.pos.trDelta, aimLocation );
		}
	}
}

int BotGetAimPredictionTime( gentity_t *self )
{
	return ( 10 - self->botMind->botSkill.level ) * 100 * MAX( ( ( float ) rand() ) / RAND_MAX, 0.5f );
}

void BotPredictPosition( gentity_t *self, gentity_t *predict, vec3_t pos, int time )
{
	botTarget_t target;
	vec3_t aimLoc;
	BotSetTarget( &target, predict, NULL );
	BotGetIdealAimLocation( self, target, aimLoc );
	VectorMA( aimLoc, time / 1000.0f, predict->s.apos.trDelta, pos );
}

void BotAimAtEnemy( gentity_t *self )
{
	vec3_t desired;
	vec3_t current;
	vec3_t viewOrigin;
	vec3_t newAim;
	vec3_t angles;
	int i;
	float frac;
	gentity_t *enemy = self->botMind->goal.ent;

	if ( self->botMind->futureAimTime < level.time )
	{
		int predictTime = self->botMind->futureAimTimeInterval = BotGetAimPredictionTime( self );
		BotPredictPosition( self, enemy, self->botMind->futureAim, predictTime );
		self->botMind->futureAimTime = level.time + predictTime;
	}

	BG_GetClientViewOrigin( &self->client->ps, viewOrigin );
	VectorSubtract( self->botMind->futureAim, viewOrigin, desired );
	VectorNormalize( desired );
	AngleVectors( self->client->ps.viewangles, current, NULL, NULL );

	frac = ( 1.0f - ( ( float ) ( self->botMind->futureAimTime - level.time ) ) / self->botMind->futureAimTimeInterval );
	VectorLerp( current, desired, frac, newAim );

	VectorSet( self->client->ps.delta_angles, 0, 0, 0 );
	vectoangles( newAim, angles );

	for ( i = 0; i < 3; i++ )
	{
		self->botMind->cmdBuffer.angles[ i ] = ANGLE2SHORT( angles[ i ] );
	}
}

void BotAimAtLocation( gentity_t *self, vec3_t target )
{
	vec3_t aimVec, aimAngles, viewBase;
	int i;
	usercmd_t *rAngles = &self->botMind->cmdBuffer;

	if ( ! ( self && self->client ) )
	{
		return;
	}

	BG_GetClientViewOrigin( &self->client->ps, viewBase );
	VectorSubtract( target, viewBase, aimVec );

	vectoangles( aimVec, aimAngles );

	VectorSet( self->client->ps.delta_angles, 0.0f, 0.0f, 0.0f );

	for ( i = 0; i < 3; i++ )
	{
		aimAngles[i] = ANGLE2SHORT( aimAngles[i] );
	}

	//save bandwidth
	SnapVector( aimAngles );
	rAngles->angles[0] = aimAngles[0];
	rAngles->angles[1] = aimAngles[1];
	rAngles->angles[2] = aimAngles[2];
}

void BotSlowAim( gentity_t *self, vec3_t target, float slowAmount )
{
	vec3_t viewBase;
	vec3_t aimVec, forward;
	vec3_t skilledVec;
	float length;
	float slow;
	float cosAngle;

	if ( !( self && self->client ) )
	{
		return;
	}
	//clamp to 0-1
	slow = Com_Clamp( 0.1f, 1.0, slowAmount );

	//get the point that the bot is aiming from
	BG_GetClientViewOrigin( &self->client->ps, viewBase );

	//get the Vector from the bot to the enemy (ideal aim Vector)
	VectorSubtract( target, viewBase, aimVec );
	length = VectorNormalize( aimVec );

	//take the current aim Vector
	AngleVectors( self->client->ps.viewangles, forward, NULL, NULL );

	cosAngle = DotProduct( forward, aimVec );
	cosAngle = ( cosAngle + 1 ) / 2;
	cosAngle = 1 - cosAngle;
	cosAngle = Com_Clamp( 0.1, 0.5, cosAngle );
	VectorLerp( forward, aimVec, slow * ( cosAngle ), skilledVec );

	//now find a point to return, this point will be aimed at
	VectorMA( viewBase, length, skilledVec, target );
}

float BotAimNegligence( gentity_t *self, botTarget_t target )
{
	vec3_t forward;
	vec3_t ideal;
	vec3_t targetPos;
	vec3_t viewPos;
	float angle;
	AngleVectors( self->client->ps.viewangles, forward, NULL, NULL );
	BG_GetClientViewOrigin( &self->client->ps, viewPos );
	BotGetIdealAimLocation( self, target, targetPos );
	VectorSubtract( targetPos, viewPos, ideal );
	VectorNormalize( ideal );
	angle = DotProduct( ideal, forward );
	angle = RAD2DEG( acos( angle ) );
	return angle;
}

/*
========================
Bot Team Querys
========================
*/

int FindBots( int *botEntityNumbers, int maxBots, team_t team )
{
	gentity_t *testEntity;
	int numBots = 0;
	int i;
	memset( botEntityNumbers, 0, sizeof( int )*maxBots );
	for ( i = 0; i < MAX_CLIENTS; i++ )
	{
		testEntity = &g_entities[i];
		if ( testEntity->r.svFlags & SVF_BOT )
		{
			if ( testEntity->client->pers.teamSelection == team && numBots < maxBots )
			{
				botEntityNumbers[numBots++] = i;
			}
		}
	}
	return numBots;
}

qboolean PlayersBehindBotInSpawnQueue( gentity_t *self )
{
	//this function only checks if there are Humans in the SpawnQueue
	//which are behind the bot
	int i;
	int botPos = 0, lastPlayerPos = 0;
	spawnQueue_t *sq;

	if ( self->client->pers.teamSelection == TEAM_HUMANS )
	{
		sq = &level.humanSpawnQueue;
	}

	else if ( self->client->pers.teamSelection == TEAM_ALIENS )
	{
		sq = &level.alienSpawnQueue;
	}
	else
	{
		return qfalse;
	}

	i = sq->front;

	if ( G_GetSpawnQueueLength( sq ) )
	{
		do
		{
			if ( !( g_entities[sq->clients[ i ]].r.svFlags & SVF_BOT ) )
			{
				if ( i < sq->front )
				{
					lastPlayerPos = i + MAX_CLIENTS - sq->front;
				}
				else
				{
					lastPlayerPos = i - sq->front;
				}
			}

			if ( sq->clients[ i ] == self->s.number )
			{
				if ( i < sq->front )
				{
					botPos = i + MAX_CLIENTS - sq->front;
				}
				else
				{
					botPos = i - sq->front;
				}
			}

			i = QUEUE_PLUS1( i );
		}
		while ( i != QUEUE_PLUS1( sq->back ) );
	}

	if ( botPos < lastPlayerPos )
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

qboolean BotTeamateHasWeapon( gentity_t *self, int weapon )
{
	int botNumbers[MAX_CLIENTS];
	int i;
	int numBots = FindBots( botNumbers, MAX_CLIENTS, ( team_t ) self->client->ps.stats[STAT_TEAM] );

	for ( i = 0; i < numBots; i++ )
	{
		gentity_t *bot = &g_entities[botNumbers[i]];
		if ( bot == self )
		{
			continue;
		}
		if ( BG_InventoryContainsWeapon( weapon, bot->client->ps.stats ) )
		{
			return qtrue;
		}
	}
	return qfalse;
}

/*
========================
Misc Bot Stuff
========================
*/
void BotFireWeapon( weaponMode_t mode, usercmd_t *botCmdBuffer )
{
	if ( mode == WPM_PRIMARY )
	{
		usercmdPressButton( botCmdBuffer->buttons, BUTTON_ATTACK );
	}
	else if ( mode == WPM_SECONDARY )
	{
		usercmdPressButton( botCmdBuffer->buttons, BUTTON_ATTACK2 );
	}
	else if ( mode == WPM_TERTIARY )
	{
		usercmdPressButton( botCmdBuffer->buttons, BUTTON_USE_HOLDABLE );
	}
}
void BotClassMovement( gentity_t *self, qboolean inAttackRange )
{
	usercmd_t *botCmdBuffer = &self->botMind->cmdBuffer;

	switch ( self->client->ps.stats[STAT_CLASS] )
	{
		case PCL_ALIEN_LEVEL0:
			BotStrafeDodge( self );
			break;
		case PCL_ALIEN_LEVEL1:
		case PCL_ALIEN_LEVEL1_UPG:
			if ( BotTargetIsPlayer( self->botMind->goal ) && ( self->botMind->goal.ent->client->ps.stats[STAT_STATE] & SS_GRABBED ) && inAttackRange )
			{
				if ( self->botMind->botSkill.level == 10 )
				{
					BotStandStill( self );
					BotStrafeDodge( self );//only move if skill == 10 because otherwise we wont aim fast enough to not lose grab
				}
				else
				{
					BotStandStill( self );
				}
			}
			else
			{
				BotStrafeDodge( self );
			}
			break;
		case PCL_ALIEN_LEVEL2:
		case PCL_ALIEN_LEVEL2_UPG:
			if ( self->botMind->directPathToGoal )
			{
				if ( self->client->time1000 % 300 == 0 )
				{
					BotJump( self );
				}
				BotStrafeDodge( self );
			}
			break;
		case PCL_ALIEN_LEVEL3:
			break;
		case PCL_ALIEN_LEVEL3_UPG:
			if ( BotGetTargetType( self->botMind->goal ) == ET_BUILDABLE && self->client->ps.ammo > 0
			        && inAttackRange )
			{
				//dont move when sniping buildings
				BotStandStill( self );
			}
			break;
		case PCL_ALIEN_LEVEL4:
			//use rush to approach faster
			if ( !inAttackRange )
			{
				BotFireWeapon( WPM_SECONDARY, botCmdBuffer );
			}
			break;
		default:
			break;
	}
}

float CalcAimPitch( gentity_t *self, botTarget_t target, vec_t launchSpeed )
{
	vec3_t startPos;
	vec3_t targetPos;
	float initialHeight;
	vec3_t forward, right, up;
	vec3_t muzzle;
	float distance2D;
	float x, y, v, g;
	float check;
	float angle1, angle2, angle;

	BotGetTargetPos( target, targetPos );
	AngleVectors( self->s.origin, forward, right, up );
	CalcMuzzlePoint( self, forward, right, up, muzzle );
	VectorCopy( muzzle, startPos );

	//project everything onto a 2D plane with initial position at (0,0)
	initialHeight = startPos[2];
	targetPos[2] -= initialHeight;
	startPos[2] -= initialHeight;
	distance2D = sqrt( Square( startPos[0] - targetPos[0] ) + Square( startPos[1] - targetPos[1] ) );
	targetPos[0] = distance2D;

	//for readability's sake
	x = targetPos[0];
	y = targetPos[2];
	v = launchSpeed;
	g = self->client->ps.gravity;

	//make sure we won't get NaN
	check = Square( Square( v ) ) - g * ( g * Square( x ) + 2 * y * Square( v ) );

	//as long as we will get NaN, increase velocity to compensate
	//This is better than returning some failure value because it gives us the best launch angle possible, even if it wont hit in the end.
	while ( check < 0 )
	{
		v += 5;
		check = Square( Square( v ) ) - g * ( g * Square( x ) + 2 * y * Square( v ) );
	}
	
	//calculate required angle of launch
	angle1 = atanf( ( Square( v ) + sqrt( check ) ) / ( g * x ) );
	angle2 = atanf( ( Square( v ) - sqrt( check ) ) / ( g * x ) );

	//take the smaller angle
	angle = ( angle1 < angle2 ) ? angle1 : angle2;

	//convert to degrees (ps.viewangles units)
	angle = RAD2DEG( angle );
	return angle;
}
float CalcPounceAimPitch( gentity_t *self, botTarget_t target )
{
	vec_t speed = ( self->client->ps.stats[STAT_CLASS] == PCL_ALIEN_LEVEL3 ) ? LEVEL3_POUNCE_JUMP_MAG : LEVEL3_POUNCE_JUMP_MAG_UPG;
	return CalcAimPitch( self, target, speed );

	//in usrcmd angles, a positive angle is down, so multiply angle by -1
	// botCmdBuffer->angles[PITCH] = ANGLE2SHORT(-angle);
}
float CalcBarbAimPitch( gentity_t *self, botTarget_t target )
{
	vec_t speed = LEVEL3_BOUNCEBALL_SPEED;
	return CalcAimPitch( self, target, speed );

	//in usrcmd angles, a positive angle is down, so multiply angle by -1
	//botCmdBuffer->angles[PITCH] = ANGLE2SHORT(-angle);
}

void BotFireWeaponAI( gentity_t *self )
{
	float distance;
	vec3_t targetPos;
	vec3_t forward, right, up;
	vec3_t muzzle;
	trace_t trace;
	usercmd_t *botCmdBuffer = &self->botMind->cmdBuffer;

	AngleVectors( self->client->ps.viewangles, forward, right, up );
	CalcMuzzlePoint( self, forward, right, up, muzzle );
	BotGetIdealAimLocation( self, self->botMind->goal, targetPos );

	trap_Trace( &trace, muzzle, NULL, NULL, targetPos, -1, MASK_SHOT );
	distance = Distance( muzzle, trace.endpos );
	switch ( self->s.weapon )
	{
		case WP_ABUILD:
			if ( distance <= ABUILDER_CLAW_RANGE )
			{
				BotFireWeapon( WPM_SECONDARY, botCmdBuffer );
			}
			else
			{
				usercmdPressButton( botCmdBuffer->buttons, BUTTON_GESTURE );    //make cute granger sounds to ward off the would be attackers
			}
			break;
		case WP_ABUILD2:
			if ( distance <= ABUILDER_CLAW_RANGE )
			{
				BotFireWeapon( WPM_SECONDARY, botCmdBuffer );    //swipe
			}
			else
			{
				BotFireWeapon( WPM_TERTIARY, botCmdBuffer );    //blob launcher
			}
			break;
		case WP_ALEVEL0:
			break; //auto hit
		case WP_ALEVEL1:
			BotFireWeapon( WPM_PRIMARY, botCmdBuffer ); //basi swipe
			break;
		case WP_ALEVEL1_UPG:
			if ( distance <= LEVEL1_CLAW_U_RANGE )
			{
				BotFireWeapon( WPM_PRIMARY, botCmdBuffer );    //basi swipe
			}
			/*
			else
			BotFireWeapn(WPM_SECONDARY,botCmdBuffer); //basi poisen
			*/
			break;
		case WP_ALEVEL2:
			BotFireWeapon( WPM_PRIMARY, botCmdBuffer ); //mara swipe
			break;
		case WP_ALEVEL2_UPG:
			if ( distance <= LEVEL2_CLAW_U_RANGE )
			{
				BotFireWeapon( WPM_PRIMARY, botCmdBuffer );    //mara swipe
			}
			else
			{
				BotFireWeapon( WPM_SECONDARY, botCmdBuffer );    //mara lightning
			}
			break;
		case WP_ALEVEL3:
			if ( distance > LEVEL3_CLAW_RANGE && self->client->ps.stats[ STAT_MISC ] < LEVEL3_POUNCE_TIME )
			{
				botCmdBuffer->angles[PITCH] = ANGLE2SHORT( -CalcPounceAimPitch( self, self->botMind->goal ) ); //compute and apply correct aim pitch to hit target
				BotFireWeapon( WPM_SECONDARY, botCmdBuffer ); //goon pounce
			}
			else
			{
				BotFireWeapon( WPM_PRIMARY, botCmdBuffer );    //goon chomp
			}
			break;
		case WP_ALEVEL3_UPG:
			if ( self->client->ps.ammo > 0 && distance > LEVEL3_CLAW_UPG_RANGE )
			{
				botCmdBuffer->angles[PITCH] = ANGLE2SHORT( -CalcBarbAimPitch( self, self->botMind->goal ) ); //compute and apply correct aim pitch to hit target
				BotFireWeapon( WPM_TERTIARY, botCmdBuffer ); //goon barb
			}
			else if ( distance > LEVEL3_CLAW_UPG_RANGE && self->client->ps.stats[ STAT_MISC ] < LEVEL3_POUNCE_TIME_UPG )
			{
				botCmdBuffer->angles[PITCH] = ANGLE2SHORT( -CalcPounceAimPitch( self, self->botMind->goal ) ); //compute and apply correct aim pitch to hit target
				BotFireWeapon( WPM_SECONDARY, botCmdBuffer ); //goon pounce
			}
			else
			{
				BotFireWeapon( WPM_PRIMARY, botCmdBuffer );    //goon chomp
			}
			break;
		case WP_ALEVEL4:
			if ( distance > LEVEL4_CLAW_RANGE && self->client->ps.stats[STAT_MISC] < LEVEL4_TRAMPLE_CHARGE_MAX )
			{
				BotFireWeapon( WPM_SECONDARY, botCmdBuffer );    //rant charge
			}
			else
			{
				BotFireWeapon( WPM_PRIMARY, botCmdBuffer );    //rant swipe
			}
			break;
		case WP_LUCIFER_CANNON:
			if ( self->client->ps.stats[STAT_MISC] < LCANNON_CHARGE_TIME_MAX * Com_Clamp( 0.5, 1, random() ) )
			{
				BotFireWeapon( WPM_PRIMARY, botCmdBuffer );
			}
			break;
		default:
			BotFireWeapon( WPM_PRIMARY, botCmdBuffer );
	}
}

qboolean BotEvolveToClass( gentity_t *ent, class_t newClass )
{
	int clientNum;
	int i;
	vec3_t infestOrigin;
	class_t currentClass = ent->client->pers.classSelection;
	int numLevels;
	int entityList[ MAX_GENTITIES ];
	vec3_t range = { AS_OVER_RT3, AS_OVER_RT3, AS_OVER_RT3 };
	vec3_t mins, maxs;
	int num;
	gentity_t *other;

	if ( ent->client->ps.stats[ STAT_HEALTH ] <= 0 )
	{
		return qfalse;
	}

	clientNum = ent->client - level.clients;

	//if we are not currently spectating, we are attempting evolution
	if ( ent->client->pers.classSelection != PCL_NONE )
	{
		if ( ( ent->client->ps.stats[ STAT_STATE ] & SS_WALLCLIMBING ) )
		{
			ent->client->pers.cmd.upmove = 0;
		}

		//check there are no humans nearby
		VectorAdd( ent->client->ps.origin, range, maxs );
		VectorSubtract( ent->client->ps.origin, range, mins );

		num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
		for ( i = 0; i < num; i++ )
		{
			other = &g_entities[ entityList[ i ] ];

			if ( ( other->client && other->client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS ) ||
			        ( other->s.eType == ET_BUILDABLE && other->buildableTeam == TEAM_HUMANS ) )
			{
				return qfalse;
			}
		}

		if ( !G_Overmind() )
		{
			return qfalse;
		}

		numLevels = BG_ClassCanEvolveFromTo( currentClass, newClass, ( short )ent->client->ps.persistant[ PERS_CREDIT ], g_alienStage.integer, 0 );

		if ( G_RoomForClassChange( ent, newClass, infestOrigin ) )
		{
			//...check we can evolve to that class
			if ( numLevels >= 0 &&
			        BG_ClassAllowedInStage( newClass, ( stage_t )g_alienStage.integer ) &&
			        BG_ClassIsAllowed( newClass ) )
			{

				ent->client->pers.evolveHealthFraction = ( float )ent->client->ps.stats[ STAT_HEALTH ] /
				        ( float )BG_Class( currentClass )->health;

				if ( ent->client->pers.evolveHealthFraction < 0.0f )
				{
					ent->client->pers.evolveHealthFraction = 0.0f;
				}
				else if ( ent->client->pers.evolveHealthFraction > 1.0f )
				{
					ent->client->pers.evolveHealthFraction = 1.0f;
				}

				//remove credit
				G_AddCreditToClient( ent->client, -( short )numLevels, qtrue );
				ent->client->pers.classSelection = newClass;
				BotSetNavmesh( ent, newClass );
				ClientUserinfoChanged( clientNum, qfalse );
				VectorCopy( infestOrigin, ent->s.pos.trBase );
				ClientSpawn( ent, ent, ent->s.pos.trBase, ent->s.apos.trBase );

				//trap_SendServerCommand( -1, va( "print \"evolved to %s\n\"", classname) );

				return qtrue;
			}
			else
				//trap_SendServerCommand( -1, va( "print \"Not enough evos to evolve to %s\n\"", classname) );
			{
				return qfalse;
			}
		}
		else
		{
			return qfalse;
		}
	}
	return qfalse;
}

//Cmd_Buy_f ripoff, weapon version
void BotBuyWeapon( gentity_t *self, weapon_t weapon )
{
	if ( weapon != WP_NONE )
	{
		//already got this?
		if ( BG_InventoryContainsWeapon( weapon, self->client->ps.stats ) )
		{
			return;
		}

		// Only humans can buy stuff
		if ( BG_Weapon( weapon )->team != TEAM_HUMANS )
		{
			return;
		}

		//are we /allowed/ to buy this?
		if ( !BG_Weapon( weapon )->purchasable )
		{
			return;
		}

		//are we /allowed/ to buy this?
		if ( !BG_WeaponAllowedInStage( weapon, ( stage_t )g_humanStage.integer ) || !BG_WeaponIsAllowed( weapon ) )
		{
			return;
		}

		//can afford this?
		if ( BG_Weapon( weapon )->price > ( short )self->client->pers.credit )
		{
			return;
		}

		//have space to carry this?
		if ( BG_Weapon( weapon )->slots & BG_SlotsForInventory( self->client->ps.stats ) )
		{
			return;
		}

		// In some instances, weapons can't be changed
		if ( !BG_PlayerCanChangeWeapon( &self->client->ps ) )
		{
			return;
		}

		self->client->ps.stats[ STAT_WEAPON ] = weapon;
		self->client->ps.ammo = BG_Weapon( weapon )->maxAmmo;
		self->client->ps.clips = BG_Weapon( weapon )->maxClips;

		if ( BG_Weapon( weapon )->usesEnergy &&
		        BG_InventoryContainsUpgrade( UP_BATTPACK, self->client->ps.stats ) )
		{
			self->client->ps.ammo *= BATTPACK_MODIFIER;
		}

		G_ForceWeaponChange( self, weapon );

		//set build delay/pounce etc to 0
		self->client->ps.stats[ STAT_MISC ] = 0;

		//subtract from funds
		G_AddCreditToClient( self->client, -( short )BG_Weapon( weapon )->price, qfalse );
	}
	else
	{
		return;
	}
	//update ClientInfo
	ClientUserinfoChanged( self->client->ps.clientNum, qfalse );
}
void BotBuyUpgrade( gentity_t *self, upgrade_t upgrade )
{
	qboolean  energyOnly = qfalse;

	// Only give energy from reactors or repeaters
	if ( upgrade == UP_AMMO &&
	        BG_Weapon( ( weapon_t )self->client->ps.stats[ STAT_WEAPON ] )->usesEnergy &&
	        ( G_BuildableRange( self->client->ps.origin, 100, BA_H_REACTOR ) ||
	          G_BuildableRange( self->client->ps.origin, 100, BA_H_REPEATER ) ) )
	{
		energyOnly = qtrue;
	}
	else if ( G_BuildableRange( self->client->ps.origin, 100, BA_H_ARMOURY ) )
	{
		energyOnly = qfalse;
	}
	else if ( upgrade == UP_AMMO && BG_Weapon( ( weapon_t )self->client->ps.weapon )->usesEnergy )
	{
		return;
	}

	if ( upgrade != UP_NONE )
	{
		//already got this?
		if ( BG_InventoryContainsUpgrade( upgrade, self->client->ps.stats ) )
		{
			return;
		}

		//can afford this?
		if ( BG_Upgrade( upgrade )->price > ( short )self->client->pers.credit )
		{
			return;
		}

		//have space to carry this?
		if ( BG_Upgrade( upgrade )->slots & BG_SlotsForInventory( self->client->ps.stats ) )
		{
			return;
		}

		// Only humans can buy stuff
		if ( BG_Upgrade( upgrade )->team != TEAM_HUMANS )
		{
			return;
		}

		//are we /allowed/ to buy this?
		if ( !BG_Upgrade( upgrade )->purchasable )
		{
			return;
		}

		//are we /allowed/ to buy this?
		if ( !BG_UpgradeAllowedInStage( upgrade, ( stage_t )g_humanStage.integer ) || !BG_UpgradeIsAllowed( upgrade ) )
		{
			return;
		}

		if ( upgrade == UP_AMMO )
		{
			G_GiveClientMaxAmmo( self, energyOnly );
		}
		else
		{
			if ( upgrade == UP_BATTLESUIT )
			{
				vec3_t newOrigin;

				if ( !G_RoomForClassChange( self, PCL_HUMAN_BSUIT, newOrigin ) )
				{
					return;
				}
				VectorCopy( newOrigin, self->client->ps.origin );
				self->client->ps.stats[ STAT_CLASS ] = PCL_HUMAN_BSUIT;
				self->client->pers.classSelection = PCL_HUMAN_BSUIT;
				BotSetNavmesh( self, PCL_HUMAN_BSUIT );
				self->client->ps.eFlags ^= EF_TELEPORT_BIT;
			}

			//add to inventory
			BG_AddUpgradeToInventory( upgrade, self->client->ps.stats );
		}

		if ( upgrade == UP_BATTPACK )
		{
			G_GiveClientMaxAmmo( self, qtrue );
		}

		//subtract from funds
		G_AddCreditToClient( self->client, -( short )BG_Upgrade( upgrade )->price, qfalse );
	}
	else
	{
		return;
	}

	//update ClientInfo
	ClientUserinfoChanged( self->client->ps.clientNum, qfalse );
}
void BotSellWeapons( gentity_t *self )
{
	weapon_t selected = BG_GetPlayerWeapon( &self->client->ps );
	int i;

	//no armoury nearby
	if ( !G_BuildableRange( self->client->ps.origin, 100, BA_H_ARMOURY ) )
	{
		return;
	}

	if ( !BG_PlayerCanChangeWeapon( &self->client->ps ) )
	{
		return;
	}

	//sell weapons
	for ( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
	{
		//guard against selling the HBUILD weapons exploit
		if ( i == WP_HBUILD && self->client->ps.stats[ STAT_MISC ] > 0 )
		{
			continue;
		}

		if ( BG_InventoryContainsWeapon( i, self->client->ps.stats ) &&
		        BG_Weapon( ( weapon_t )i )->purchasable )
		{
			self->client->ps.stats[ STAT_WEAPON ] = WP_NONE;

			//add to funds
			G_AddCreditToClient( self->client, ( short )BG_Weapon( ( weapon_t ) i )->price, qfalse );
		}

		//if we have this weapon selected, force a new selection
		if ( i == selected )
		{
			G_ForceWeaponChange( self, WP_NONE );
		}
	}
}
void BotSellAll( gentity_t *self )
{
	int i;

	//no armoury nearby
	if ( !G_BuildableRange( self->client->ps.origin, 100, BA_H_ARMOURY ) )
	{
		return;
	}
	BotSellWeapons( self );

	//sell upgrades
	for ( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
	{
		//remove upgrade if carried
		if ( BG_InventoryContainsUpgrade( i, self->client->ps.stats ) &&
		        BG_Upgrade( ( upgrade_t )i )->purchasable )
		{

			// shouldn't really need to test for this, but just to be safe
			if ( i == UP_BATTLESUIT )
			{
				vec3_t newOrigin;

				if ( !G_RoomForClassChange( self, PCL_HUMAN, newOrigin ) )
				{
					continue;
				}
				VectorCopy( newOrigin, self->client->ps.origin );
				self->client->ps.stats[ STAT_CLASS ] = PCL_HUMAN;
				self->client->pers.classSelection = PCL_HUMAN;
				self->client->ps.eFlags ^= EF_TELEPORT_BIT;
				BotSetNavmesh( self, PCL_HUMAN );
			}

			BG_RemoveUpgradeFromInventory( i, self->client->ps.stats );

			if ( i == UP_BATTPACK )
			{
				G_GiveClientMaxAmmo( self, qtrue );
			}

			//add to funds
			G_AddCreditToClient( self->client, ( short )BG_Upgrade( ( upgrade_t )i )->price, qfalse );
		}
	}
	//update ClientInfo
	ClientUserinfoChanged( self->client->ps.clientNum, qfalse );
}

void BotSetSkillLevel( gentity_t *self, int skill )
{
	self->botMind->botSkill.level = skill;
	//different aim for different teams
	if ( self->botMind->botTeam == TEAM_HUMANS )
	{
		self->botMind->botSkill.aimSlowness = ( float ) skill / 10;
		self->botMind->botSkill.aimShake = ( int ) ( 10 - skill );
	}
	else
	{
		self->botMind->botSkill.aimSlowness = ( float ) skill / 10;
		self->botMind->botSkill.aimShake = ( int ) ( 10 - skill );
	}
}

void BotPain( gentity_t *self, gentity_t *attacker, int damage )
{
	if ( BotGetEntityTeam( attacker ) != TEAM_NONE && BotGetEntityTeam( attacker ) != self->client->ps.stats[ STAT_TEAM ] )
	{
		if ( attacker->s.eType == ET_PLAYER )
		{
			self->botMind->bestEnemy.ent = attacker;
			self->botMind->bestEnemy.distance = Distance( self->s.origin, attacker->s.origin );
			self->botMind->enemyLastSeen = level.time;
			self->botMind->timeFoundEnemy = level.time - g_bot_reactiontime.integer; // alert immediately
		}
	}
}

void BotSearchForEnemy( gentity_t *self )
{
	if ( !self->botMind->bestEnemy.ent || level.time - self->botMind->enemyLastSeen > g_bot_chasetime.integer )
	{
		botTarget_t target;
		gentity_t *enemy = BotFindBestEnemy( self );

		if ( enemy )
		{
			BotSetTarget( &target, enemy, NULL );

			if ( enemy->s.eType != ET_PLAYER || ( enemy->s.eType == ET_PLAYER 
				&& ( self->client->ps.stats[ STAT_TEAM ] == TEAM_ALIENS || BotAimNegligence( self, target ) <= g_bot_fov.value / 2 ) ) )
			{
				self->botMind->bestEnemy.ent = enemy;
				self->botMind->bestEnemy.distance = Distance( self->s.origin, enemy->s.origin );
				self->botMind->enemyLastSeen = level.time;
				self->botMind->timeFoundEnemy = level.time;
			}
		}
		else
		{
			self->botMind->bestEnemy.ent = NULL;
		}
	}
}
