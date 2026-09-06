#include "Sources/Animation/Animation.hpp"
#include "Sources/Camera.hpp"
#include "Sources/Ecs/Ecs.hpp"
#include "Sources/Models/Models.hpp"
#include "Sources/Renderer/Render.hpp"

#include <lwcgl/context.h>
#include <lwcgl/lwcgl.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

class Example
{
private:
    Renderer::Rasterizer *renderer_ = new Renderer::Rasterizer();
    Camera::Controller *camera_controller_ = new Camera::Controller();
    Animation::System *animation_system_ = new Animation::System();
    Ecs::World *world_ = new Ecs::World();
    Ecs::Entity camera_ = Ecs::INVALID_ENTITY;

public:
    Example(const char* _title, const std::vector<int> _dim)
    {
        lwcglInstallFastRuntime();

        lwcglSetContextVersion(4, 3);
        lwcglSetContextProfile(LWCGL_CONTEXT_COMPATIBILITY_PROFILE);

        Display.setDisplayMode(new DisplayMode(_dim[0], _dim[1]));
        Display.create();
		Display.setTitle(_title);

        Keyboard.create();
        Mouse.create();

        renderer_->init();
    }

    ~Example()
    {
        renderer_->shutdown();
        Models::clearCache();
        Mouse.destroy();
        Keyboard.destroy();
        Display.destroy();

        delete animation_system_;
        delete camera_controller_;
        delete renderer_;
        delete world_;
    }

    static inline int run(int argc, char **argv)
    {
        Example *e = new Example("Test", {1280, 720});

        int _framebuffer_width  = std::max(Display.getWidth(),  1),
            _framebuffer_height = std::max(Display.getHeight(), 1);

        e->renderer_->resize(_framebuffer_width, _framebuffer_height);

        e->camera_ = e->world_->createEntity();
        e->world_->add<Renderer::Transform>(e->camera_, Renderer::Transform{
            .position = {.x = 0.0f, .y = 1.5f, .z = 5.0f},
            .rotation = {.x = 0.0f, .y = 0.0f, .z = 0.0f},
            .scale    = {.x = 1.0f, .y = 1.0f, .z = 1.0f},
        });
        e->world_->add<Camera::CameraComponent>(e->camera_, Camera::CameraComponent{60.0f, 0.1f, true});

        std::string _error;
        const std::vector<Models::ModelHandle> _models = {
			Models::load("Assets/Sponza/sponza.obj",   &_error),
			Models::load("Assets/Glock17/Glock17.fbx", &_error),
		};
		
		for (const Models::ModelHandle _model : _models) 
		{
        	if (_model == Models::INVALID_MODEL)
        	{
            	std::fprintf(stderr, "[LOG]: %s\n", _error.c_str());
            	delete e;
            	return 3;
        	}

        	if (Models::partCount(_model) == 0u)
        	{
            	std::fprintf(stderr, "[LOG]: model has no renderable parts\n");
            	delete e;
            	return 3;
			}
		}
        
		Ecs::Entity _animator = Ecs::INVALID_ENTITY;
        const Animation::SkeletonHandle _skeleton = Models::skeleton(_models[1]);
        if (_skeleton != Animation::INVALID_SKELETON)
        {
            _animator = e->world_->createEntity();

            Animation::AnimatorComponent animator;
            animator.skeleton = _skeleton;

            Animation::ClipHandle initial_clip = Animation::INVALID_CLIP;
            if (argc > 2) 
				initial_clip = Models::animation(_models[1], argv[2]);
            
			if (initial_clip == Animation::INVALID_CLIP 
				&& Models::animationCount(_models[1]) != 0u) {
			
                initial_clip = Models::animation(_models[1], 0u);
            }
            
			if (initial_clip != Animation::INVALID_CLIP) {
                Animation::play(animator, initial_clip);
            }

            e->world_->add<Animation::AnimatorComponent>(_animator, std::move(animator));
        }

		for (const Models::ModelHandle _model : _models) 
		{
        	for (std::size_t i = 0; i < Models::partCount(_model); ++i)
        	{
            	const Models::ModelPart *part = Models::part(_model, i);
            	if (!part) continue;

            	const Ecs::Entity _entity = e->world_->createEntity();
            	e->world_->add<Renderer::Transform>(_entity, Renderer::Transform{});

            	e->world_->add<Renderer::MeshComponent>(
                	_entity,
               		Renderer::MeshComponent{part->mesh, part->material}
            	);

            	e->world_->add<Renderer::RenderableComponent>(
                	_entity,
                	Renderer::RenderableComponent{true}
            	);

            	if (_animator != Ecs::INVALID_ENTITY) 
				{
            	    e->world_->add<Animation::SkinBindingComponent>(
                    	_entity,
                    	Animation::SkinBindingComponent{_animator}
                	);
            	}
        	}
		}

        using Clock = std::chrono::steady_clock;
        auto _previous = Clock::now();

        while (!Display.isCloseRequested())
        {
            Display.processMessages();
            if (Keyboard.isKeyDown(Keyboard.KEY_ESCAPE)) break;

            const auto now = Clock::now();
            const float delta_seconds = std::chrono::duration<float>(now - _previous).count();
            _previous = now;
            const float frame_delta = std::min(delta_seconds, 0.1f);

            e->animation_system_->update(*e->world_, frame_delta);
            e->camera_controller_->update(*e->world_, frame_delta);

            const int width  = std::max(Display.getWidth(), 1);
            const int height = std::max(Display.getHeight(), 1);

            if ((width != _framebuffer_width) || height != _framebuffer_height)
            {
                _framebuffer_height = height;
                _framebuffer_width  = width;
                e->renderer_->resize(width, height);
            }

            e->renderer_->render(*e->world_);
            Display.updateNoMessages();
        }

        delete e;
        return 0;
    }
};

int main(int argc, char **argv)
{
    return Example::run(argc, argv);
}
