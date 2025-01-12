#include <ecs/systems/RendererSystem.h>

#include <ecs/EntityMemoryPool.h>

#include <ecs/components/PositionComponent.h>
#include <ecs/components/SizeComponent.h>
#include <ecs/components/TextureComponent.h>

#include <graphics/TextureManager.h>

namespace ecs {

static void renderTexture(
    SDL_Texture* tex,
    SDL_Renderer* ren,
    int x,
    int y,
    int w,
    int h)
{
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    SDL_RenderCopy(ren, tex, NULL, &dst);
}

Renderer::Renderer(
    SDL_Renderer* renderer,
    const graphics::TextureManager& texture_manager)
    : m_renderer(renderer)
    , m_texture_manager(texture_manager)
{
}

Renderer::~Renderer()
{
    SDL_DestroyRenderer(m_renderer);
}

void Renderer::update(
    EntityMemoryPool& pool,
    const std::vector<Entity>& entities)
{
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    // Sort entities by Z position
    auto sortedByZEntities = entities;
    std::sort(
        sortedByZEntities.begin(),
        sortedByZEntities.end(),
        [&pool](Entity left, Entity right) {
            int z_left
                = pool.hasComponent<ZPositionComponent>(left)
                ? pool.getComponent<ZPosition>(left).z
                : 0;

            int z_right
                = pool.hasComponent<ZPositionComponent>(right)
                ? pool.getComponent<ZPosition>(right).z
                : 0;

            if (z_left == z_right) {
                return left < right;
            }

            return z_left < z_right;
        });

    for (auto entity : sortedByZEntities) {
        if (!pool.hasComponent<PositionComponent>(entity))
            continue;
        if (!pool.hasComponent<SizeComponent>(entity))
            continue;

        const auto& position = pool.getComponent<Position>(entity);
        const auto& size = pool.getComponent<Size>(entity);

        if (pool.hasComponent<FillComponent>(entity)) {
            const auto& fill = pool.getComponent<Fill>(entity);
            SDL_SetRenderDrawColor(
                m_renderer,
                fill.colour.r,
                fill.colour.g,
                fill.colour.b,
                fill.colour.a);

            SDL_Rect rectangle {
                position.x,
                position.y,
                size.w,
                size.h
            };

            SDL_RenderFillRect(
                m_renderer,
                &rectangle);
        }

        if (pool.hasComponent<BorderComponent>(entity)) {
            const auto& border = pool.getComponent<Border>(entity);
            SDL_SetRenderDrawColor(
                m_renderer,
                border.colour.r,
                border.colour.g,
                border.colour.b,
                border.colour.a);

            SDL_Rect rectangle {
                position.x,
                position.y,
                size.w,
                size.h
            };

            SDL_RenderDrawRect(
                m_renderer,
                &rectangle);
        }

        if (pool.hasComponent<TextureComponent>(entity)) {
            const auto& texture_component = pool.getComponent<Texture>(entity);
            auto texture = m_texture_manager.getTexture(texture_component.id);
            renderTexture(texture, m_renderer, position.x, position.y, size.w, size.h);
        }
    }

    SDL_RenderPresent(m_renderer);
}

}
