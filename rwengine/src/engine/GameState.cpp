#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <objects/InstanceObject.hpp>
#include <dynamics/CollisionInstance.hpp>

BasicState::BasicState()
	: saveName { "" }
	, saveTime { 0, 0, 0, 0, 0, 0, 0, 0 }
	, islandNumber { 0 }
	, cameraPosition { }
	, gameMinuteMS { 0 }
	, lastTick { 0 }
	, gameHour { 0 }
	, gameMinute { 0 }
	, padMode { 0 }
	, timeMS { 0 }
	, timeScale { 0 }
	, timeStep { 0 }
	, timeStep_unclipped { 0 }
	, frameCounter { 0 }
	, timeStep2 { 0 }
	, framesPerUpdate { 0 }
	, timeScale2 { 0 }
	, lastWeather { 0 }
	, nextWeather { 0 }
	, forcedWeather { 0 }
	, weatherInterpolation { 0 }
	, weatherType { 0 }
	, cameraData { 0 }
	, cameraData2 { 0 }
{ }

PlayerInfo::PlayerInfo()
	: money { 0 }
	, displayedMoney { 0 }
	, hiddenPackagesCollected { 0 }
	, hiddenPackageCount { 0 }
	, neverTired { 0 }
	, fastReload { 0 }
	, thaneOfLibertyCity { 0 }
	, singlePayerHealthcare { 0 }
{ }

GameStats::GameStats()
	: playerKills { 0 }
	, otherKills { 0 }
	, carsExploded { 0 }
	, shotsHit { 0 }
	, pedTypesKilled { }
	, helicoptersDestroyed { 0 }
	, playerProgress { 0 }
	, explosiveKgsUsed { 0 }
	, bulletsFired { 0 }
	, bulletsHit { 0 }
	, carsCrushed { 0 }
	, headshots { 0 }
	, timesBusted { 0 }
	, timesHospital { 0 }
	, daysPassed { 0 }
	, mmRainfall { 0 }
	, insaneJumpMaxDistance { 0 }
	, insaneJumpMaxHeight { 0 }
	, insaneJumpMaxFlips { 0 }
	, insangeJumpMaxRotation { 0 }
	, bestStunt { 0 }
	, uniqueStuntsFound { 0 }
	, uniqueStuntsTotal { 0 }
	, missionAttempts { 0 }
	, missionsPassed { 0 }
	, passengersDroppedOff { 0 }
	, taxiRevenue { 0 }
	, portlandPassed { 0 }
	, stauntonPassed { 0 }
	, shoresidePassed { 0 }
	, bestTurismoTime { 0 }
	, distanceWalked { 0 }
	, distanceDriven { 0 }
	, patriotPlaygroundTime { 0 }
	, aRideInTheParkTime { 0 }
	, grippedTime { 0 }
	, multistoryMayhemTime { 0 }
	, peopleSaved { 0 }
	, criminalsKilled { 0 }
	, highestParamedicLevel { 0 }
	, firesExtinguished { 0 }
	, longestDodoFlight { 0 }
	, bombDefusalTime { 0 }
	, rampagesPassed { 0 }
	, totalRampages { 0 }
	, totalMissions { 0 }
	, highestScore { }
	, peopleKilledSinceCheckpoint { 0 }
	, peopleKilledSinceLastBustedOrWasted { 0 }
	, lastMissionGXT { "" }
{ }

GameState::GameState()
	: basic{}
	, gameTime(0.f)
	, currentProgress(0)
	, maxProgress(1)
	, maxWantedLevel(0)
	, playerObject(0)
	, scriptOnMissionFlag(nullptr)
	, fadeOut(true)
	, fadeStart(0.f)
	, fadeTime(0.f)
	, fadeSound(false)
	, skipCutscene(false)
	, isIntroPlaying(false)
	, currentCutscene(nullptr)
	, cutsceneStartTime(-1.f)
	, isCinematic(false)
	, cameraNear(0.1f)
	, cameraFixed(false)
	, cameraTarget(0)
	, world(nullptr)
	, script(nullptr)
{
	
}

int GameState::addRadarBlip(BlipData& blip)
{
	int l = 0;
	for ( auto x = radarBlips.begin(); x != radarBlips.end(); ++x )
	{
		if ( (x->first) != l )
		{
			l = x->first-1;
		}
		else
		{
			l++;
		}
	}
	
	blip.id = l;
	radarBlips.insert({l, blip});
	
	return l;
}

void GameState::removeBlip(int blip)
{
	auto it = radarBlips.find( blip );
	if ( it != radarBlips.end() )
	{
		radarBlips.erase(it);
	}
}

void GarageInfo::findDoorObject(GameWorld* world)
{
	auto mid = (min+max)/2.f;
	auto closest = std::numeric_limits<float>::max();
	for (auto& obj : world->specialModelInstances) {
		auto distance = glm::distance2(obj->getPosition(), mid);
		if (closest > distance) {
			closest = distance;
			doorObject = obj;
			closedDoorPosition = obj->getPosition();
		}
	}
	if (doorObject) {
		auto height = doorObject->model->resource->getBoundingRadius();
		openDoorPosition = closedDoorPosition + glm::vec3(0.f, 0.f, height);
		static_cast<InstanceObject*>(doorObject)->body->changeKinematic(true);
	}
}

void GarageInfo::updateDoor(float dt)
{
	if (! doorObject)
		return;

	constexpr float kDoorMoveSpeed = 1.f;
	float maxDist = kDoorMoveSpeed * dt;
	const auto& targetPos = open ? openDoorPosition : closedDoorPosition;

	auto pdelta = targetPos - doorObject->getPosition();
	const auto dist2 = glm::length2(pdelta);
	if (dist2 > (maxDist*maxDist)) {
		auto dir = glm::normalize(pdelta);
		doorObject->setPosition(doorObject->getPosition() + dir * maxDist);
	}
	else if (dist2 < 0.001f) {
		open = !open;
	}
	else {
		doorObject->setPosition(targetPos);
	}
}
