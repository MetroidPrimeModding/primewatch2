#include <vector>
#include "prime1/Prime1JsonDumper.hpp"
#include "prime1/CGameAllocator.hpp"
#include "prime1/actors/CPlayer.hpp"
#include "prime1/CWorld.hpp"
#include "prime1/CStateManager.hpp"
#include "prime1/CGameGlobalObjects.hpp"
#include "prime1/CGameState.hpp"

using namespace nlohmann;

namespace Prime1JsonDumper {
    json parseHeap() {
      json json_heap;
      CGameAllocator allocator(CGameAllocator::LOCATION);

      json_heap["size"] = allocator.heapSize.read();
      std::vector<json> json_blocks;

      CMemoryBlock block = allocator.first.deref();
      size_t blockid = 0;

      while (block.ptr() != 0 && block.size.read() != 0) {
        json json_block;
        json_block["size"] = block.size.read();
        json_block["start"] = block.ptr();
        json_blocks.push_back(json_block);

        block = block.next.deref();
        blockid++;
      }
      json_heap["blocks"] = json_blocks;
      return json_heap;
    }

    json parsePlayer() {
      json res;
      CStateManager stateManager(CStateManager::LOCATION);
      CPlayer player = stateManager.player.deref();

      res["transform"] = player.transform.matrix.json();
      res["constant_force"] = player.constantForce.json();
      res["angular_momentum"] = player.angularMomentum.json();
      res["velocity"] = player.velocity.json();
      res["angular_velocity"] = player.angularVelocity.json();
      res["momentum"] = player.momentum.json();
      res["force"] = player.force.json();
      res["impulse"] = player.impulse.json();
      res["torque"] = player.torque.json();
      res["angular_impulse"] = player.angularImpulse.json();
      res["collision_primitive"] = player.collisionPrimitive.aabb.values.json();
      res["primitive_offset"] = player.primitiveOffset.json();
      res["translation"] = player.translation.json();
      res["orientation"] = player.orientation.json();
      res["camera_state"] = player.cameraState.read();
      res["morph_state"] = player.morphState.read();
//      json_player["gun"]
      res["morphball"] = player.morphBall.deref().json();
      res["jump_state"] = player.jumpState.read();
      res["spacejump_timer"] = player.sjTimer.read();

      CPlayerState playerState = stateManager.playerState.deref().deref();
      res["health"] = playerState.healthInfo.health.read();
      res["beam"] = playerState.currentBeam.read();
      res["visor"] = playerState.currentVisor.read();
      res["suit"] = playerState.currentSuit.read();
      res["inventory"] = playerState.powerups.array.json();

      CGameGlobalObjects global(CGameGlobalObjects::LOCATION);
      CGameState gameState = global.gameState.deref();
      res["timer"] = gameState.playTime.read();
      res["framecount"] = global.frameCount.read();

      json camera_bob;
      camera_bob["transform"] = player.cameraBob.deref().cameraBobTransform.matrix.json();
      camera_bob["magnitude"] = player.cameraBob.deref().bobMagnitude.read();
      camera_bob["time_scale"] = player.cameraBob.deref().bobTimeScale.read();
      res["camera_bob"] = camera_bob;

      return res;
    }

    json parsePlayerRaw() {
      json res = parsePlayer();
      CStateManager stateManager(CStateManager::LOCATION);
      CPlayer player = stateManager.player.deref();

      res["transform"] = player.transform.rawMatrix.json();
      res["constant_force"] = player.constantForce.rawJson();
      res["angular_momentum"] = player.angularMomentum.rawJson();
      res["velocity"] = player.velocity.rawJson();
      res["angular_velocity"] = player.angularVelocity.rawJson();
      res["momentum"] = player.momentum.rawJson();
      res["force"] = player.force.rawJson();
      res["impulse"] = player.impulse.rawJson();
      res["torque"] = player.torque.rawJson();
      res["angular_impulse"] = player.angularImpulse.rawJson();
      res["collision_primitive"] = player.collisionPrimitive.aabb.rawValues.json();
      res["primitive_offset"] = player.primitiveOffset.rawJson();
      res["translation"] = player.translation.rawJson();
      res["orientation"] = player.orientation.rawJson();

      json camera_bob;
      camera_bob["transform"] = player.cameraBob.deref().cameraBobTransform.rawMatrix.json();
      camera_bob["magnitude"] = player.cameraBob.deref().rawBobMagnitude.read();
      camera_bob["time_scale"] = player.cameraBob.deref().rawBobTimeScale.read();
      res["camera_bob"] = camera_bob;

      return res;
    }

    json parseWorld() {
      json res;
      CStateManager stateManager(CStateManager::LOCATION);
      CWorld world = stateManager.world.deref();

      res["phase"] = world.phase.read();
      res["mlvl"] = world.mlvlID.read();
      res["strg"] = world.strgID.read();
      res["area"] = world.currentAreaID.read();

      return res;
    }

    json parseCamera(CGameCamera &cam);

    json parseCamera() {
      json res;
      CStateManager stateManager(CStateManager::LOCATION);

      CCameraManager cameraManager = stateManager.cameraManager.deref();
      if (cameraManager.ptr() == 0) {
        return res;
      }

      CFirstPersonCamera firstPerson = cameraManager.firstPerson.deref();
      CBallCamera ball = cameraManager.ball.deref();

      if (firstPerson.ptr() == 0) {
        return res;
      }

      res["ball"] = parseCamera(ball);
      json firstPerson_json = parseCamera(firstPerson);

      firstPerson_json["gun_follow"] = firstPerson.gunFollowXf.matrix.json();
      firstPerson_json["gun_follow_raw"] = firstPerson.gunFollowXf.rawMatrix.json();
      res["first_person"] = firstPerson_json;

      return res;
    }

    json parseCamera(CGameCamera &cam) {
      json res;
      res["watched"] = cam.watchedObject.read();
      res["perspective"] = cam.perspectiveMatrix.json();
      res["perspective_raw"] = cam.perspectiveMatrix.rawJson();
      res["transform"] = cam.transform.matrix.json();
      res["transform_raw"] = cam.transform.rawMatrix.json();
      res["transform_addr"] = cam.transform.ptr();
      res["transform_cam"] = cam.camTransform.matrix.json();
      res["transform_cam_raw"] = cam.camTransform.rawMatrix.json();
      res["current_fov"] = cam.currentFov.read();
      res["znear"] = cam.znear.read();
      res["zfar"] = cam.zfar.read();
      res["aspect"] = cam.aspect.read();
      res["fov"] = cam.fov.read();

      return res;
    }

    json parsePool() {
      CGameGlobalObjects global(CGameGlobalObjects::LOCATION);
      CSimplePool pool = global.mainPool;

      std::vector<nlohmann::json> ids;

      auto inOrder = pool.resources.root.deref().inOrder();

      transform(inOrder.begin(), inOrder.end(), back_inserter(ids), [](CSimplePool::ResourceType pair) { return pair.b.json(); });

      return json(ids);
    }

    json parsePoolBasic() {
      CGameGlobalObjects global(CGameGlobalObjects::LOCATION);
      CSimplePool pool = global.mainPool;

      std::vector<uint32_t> ids;

      auto inOrder = pool.resources.root.deref().inOrder();

      transform(inOrder.begin(), inOrder.end(), back_inserter(ids), [](CSimplePool::ResourceType pair) { return pair.a.id.read(); });

      return json(ids);
    }

    json parsePoolSummary() {
      CGameGlobalObjects global(CGameGlobalObjects::LOCATION);
      CSimplePool pool = global.mainPool;

      json res;
      res["count"] = pool.resources.size.read();

      return res;
    }

    json parseHeapStats() {
      CGameAllocator allocator(CGameAllocator::LOCATION);

      json res;

      res["heap_size"] = allocator.heapSize.read();

      uint32_t unusedCount = 0;
      uint32_t unusedSize = 0;
      uint32_t usedCount = 0;
      uint32_t usedSize = 0;

      for (auto block = allocator.first.deref(); block.ptr() != 0; block = block.next.deref()) {
        if (unusedCount + usedCount > 10000) {
          //Curses, memory was modified out from under us
          break;
        }
        if (block.dataStart.read() == 0) {
          unusedCount++;
          unusedSize += block.size.read();
        } else {
          usedCount++;
          usedSize += block.size.read();
        }
      }

      res["unused_count"] = unusedCount;
      res["unused_size"] = unusedSize;
      res["used_count"] = usedCount;
      res["used_size"] = usedSize;

      return res;
    }
};
