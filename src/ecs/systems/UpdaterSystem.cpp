#include <ecs/systems/UpdaterSystem.h>

#include <ecs/EntityMemoryPool.h>

namespace ecs {

void Updater::update(
    EntityMemoryPool& pool,
    const std::vector<Entity>& entities)
{ /*
     for (auto entity : entities) {
         const auto x_delta = sin(rand()) * 2.f;
         const auto y_delta = sin(rand()) * 2.f;
         auto& rectangle = pool.getComponent<Rectangle>(entity);

         rectangle.rectangle.x += x_delta;
         rectangle.rectangle.y += y_delta;
         if (rectangle.rectangle.x > 800) {
             rectangle.rectangle.x = 0;
         } else if (rectangle.rectangle.x < 0) {
             rectangle.rectangle.x = 800;
         }

         if (rectangle.rectangle.y > 600) {
             rectangle.rectangle.y = 0;
         } else if (rectangle.rectangle.y < 0) {
             rectangle.rectangle.y = 600;
         }
     }*/
}

}
