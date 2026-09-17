#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "LavaEngine/Application.hpp"

using namespace LavaEngine;

namespace
{
    int g_failures = 0;

#define CHECK(cond)                                                       \
    do                                                                    \
    {                                                                     \
        if (!(cond))                                                      \
        {                                                                 \
            ++g_failures;                                                 \
            std::fprintf(                                                 \
                stderr,                                                   \
                "FAIL %s:%d: %s\n",                                       \
                __FILE__,                                                 \
                __LINE__,                                                 \
                #cond                                                      \
            );                                                            \
        }                                                                 \
    } while (0)

    struct Mesh : Resource
    {
    };

    struct RecorderModule : Module
    {
        void imgui() const override {}

        void onUnload() override
        {
            g_order.push_back("module.onUnload");
        }

        ~RecorderModule() override
        {
            g_order.push_back("module.dtor");
        }

        inline static std::vector<std::string> g_order;
    };

    struct LifecycleContainer : Container
    {
        void imgui() const override {}

        void onUnload() override
        {
            RecorderModule::g_order.push_back("container.onUnload");
            Container::onUnload();
        }

        ~LifecycleContainer() override
        {
            RecorderModule::g_order.push_back("container.dtor");
        }
    };

    std::size_t positionOf(const std::string& event)
    {
        const auto it =
            std::find(
                RecorderModule::g_order.begin(),
                RecorderModule::g_order.end(),
                event
            );

        return it == RecorderModule::g_order.end()
            ? static_cast<std::size_t>(-1)
            : static_cast<std::size_t>(
                  std::distance(
                      RecorderModule::g_order.begin(),
                      it
                  )
              );
    }


    void testGenerationHandles()
    {
        ResourceRegistry reg;

        ResourceHandle h1 = reg.create<Mesh>();
        ResourceHandle h2 = reg.create<Mesh>();

        CHECK(h1.valid());
        CHECK(h1 != h2);
        CHECK(reg.contains(h1));
        CHECK(reg.size() == 2);

        reg.remove(h1);
        CHECK(!reg.contains(h1));
        CHECK(reg.get<Mesh>(h1) == nullptr);
        CHECK(reg.size() == 1);

        // Recreating at the same id must not alias the stale handle.
        ResourceHandle h3 = reg.create<Mesh>();
        CHECK(h3 != h1);
        CHECK(reg.get<Mesh>(h1) == nullptr);
        CHECK(reg.get<Mesh>(h3) != nullptr);

        ResourceHandle invalid;
        CHECK(!invalid.valid());
        CHECK(!reg.contains(invalid));

        reg.clear();
        CHECK(reg.empty());
        CHECK(reg.get<Mesh>(h3) == nullptr);
    }

    void testResourceViewLifetime()
    {
        auto view = ResourceView<Mesh>{};

        {
            ResourceRegistry reg;
            ResourceHandle h = reg.create<Mesh>();

            view = reg.view<Mesh>(h);

            CHECK(view.valid());
            CHECK(view.get() != nullptr);
            CHECK(static_cast<bool>(view));

            reg.remove(h);

            CHECK(!view.valid());
            CHECK(view.get() == nullptr);
            CHECK(!static_cast<bool>(view));
        }

        CHECK(view.expired());
        CHECK(view.get() == nullptr);
    }

    void testViewSurvivesRegistryMove()
    {
        ResourceRegistry reg;
        ResourceHandle h = reg.create<Mesh>();

        ResourceView<Mesh> view = reg.view<Mesh>(h);

        ResourceRegistry moved = std::move(reg);
        CHECK(moved.contains(h));
        CHECK(view.valid());
        CHECK(view.get() != nullptr);

        ResourceRegistry target;
        target = std::move(moved);
        CHECK(target.contains(h));
        CHECK(view.valid());
        CHECK(view.get() != nullptr);
    }

    void testAllIteration()
    {
        ResourceRegistry reg;
        ResourceHandle h = reg.create<Mesh>();

        for (const auto& [id, entry] : reg.all())
        {
            CHECK(id == h.id());
            CHECK(entry.generation == h.generation());
            CHECK(entry.resource.get() != nullptr);
        }
    }

    void testContainerImportResource()
    {
        Container exporter("exporter");
        Container importer("importer");

        ResourceHandle h = exporter.createResource<Mesh>();
        exporter.exportResource<Mesh>(h);

        ResourceView<Mesh> view =
            importer.importResource<Mesh>(exporter, h);

        CHECK(view.valid());
        CHECK(view.get() != nullptr);

        // A non-exported resource cannot be imported.
        Container other("other");
        ResourceHandle h2 = other.createResource<Mesh>();

        ResourceView<Mesh> view2 =
            importer.importResource<Mesh>(other, h2);

        CHECK(!view2.valid());
        CHECK(view2.get() == nullptr);

        // Destroying the exporter invalidates existing views.
        {
            Container dying("dying");
            ResourceHandle hd = dying.createResource<Mesh>();
            dying.exportResource<Mesh>(hd);

            view = importer.importResource<Mesh>(dying, hd);
            CHECK(view.valid());
        }

        CHECK(!view.valid());
        CHECK(view.get() == nullptr);
    }

    void testContainerMoveRepointsModules()
    {
        Container a("a");
        RecorderModule& module = a.addModule<RecorderModule>();

        Container b = std::move(a);
        CHECK(&module.getContainer() == &b);

        Container c("c");
        c = std::move(b);
        CHECK(&module.getContainer() == &c);
    }

    void testExposedBoxes()
    {
        Container c("c");

        bool callerBool = true;
        int callerInt = 7;
        float callerFloat = 1.5f;
        double callerDouble = 2.5;
        std::string callerString = "original";

        c.expose("flag", &callerBool);
        c.expose("count", &callerInt);
        c.expose("speed", &callerFloat);
        c.expose("scale", &callerDouble);
        c.expose("title", &callerString);

        CHECK(c.variables().size() == 5);

        Variable* stringVar = nullptr;
        for (Variable& v : c.variables())
        {
            if (v.name == "title")
                stringVar = &v;
        }
        CHECK(stringVar != nullptr);

        // Simulate the Inspector's widgets writing through ptr.
        *static_cast<bool*>(c.variables()[0].ptr) = false;
        *static_cast<int*>(c.variables()[1].ptr) = 42;
        *static_cast<float*>(c.variables()[2].ptr) = 99.0f;
        *static_cast<double*>(c.variables()[3].ptr) = 123.0;
        *static_cast<std::string*>(stringVar->ptr) = "edited";

        // Boxes are snapshots: the caller's memory is untouched, but the
        // Inspector's edits are visible (and stable, i.e. boxed).
        CHECK(callerBool == true);
        CHECK(callerInt == 7);
        CHECK(callerFloat == 1.5f);
        CHECK(callerDouble == 2.5);
        CHECK(callerString == "original");

        CHECK(*static_cast<std::string*>(stringVar->ptr) == "edited");
        CHECK(*static_cast<float*>(c.variables()[2].ptr) == 99.0f);
    }

    void testTeardownHooksRunBeforeDestruction()
    {
        RecorderModule::g_order.clear();

        {
            Application app;
            LifecycleContainer& world =
                app.createContainer<LifecycleContainer>();
            world.addModule<RecorderModule>();

            app.unloadGame();
        }

        const std::size_t containerHook = positionOf("container.onUnload");
        const std::size_t moduleHook = positionOf("module.onUnload");
        const std::size_t moduleDtor = positionOf("module.dtor");
        const std::size_t containerDtor = positionOf("container.dtor");

        CHECK(containerHook == 0);
        CHECK(moduleHook != static_cast<std::size_t>(-1));
        CHECK(moduleDtor != static_cast<std::size_t>(-1));
        CHECK(containerDtor != static_cast<std::size_t>(-1));
        CHECK(moduleHook < moduleDtor);
        CHECK(moduleHook < containerDtor);
    }

    void testDefaultContainerOnUnload()
    {
        RecorderModule::g_order.clear();

        Container c("plain");
        c.addModule<RecorderModule>();

        c.onUnload();

        CHECK(RecorderModule::g_order ==
              std::vector<std::string>{"module.onUnload"});
    }
} // namespace

int main()
{
    testGenerationHandles();
    testResourceViewLifetime();
    testViewSurvivesRegistryMove();
    testAllIteration();
    testContainerImportResource();
    testContainerMoveRepointsModules();
    testExposedBoxes();
    testTeardownHooksRunBeforeDestruction();
    testDefaultContainerOnUnload();

    if (g_failures == 0)
    {
        std::printf("All tests passed.\n");
        return 0;
    }

    std::fprintf(stderr, "%d check(s) failed.\n", g_failures);
    return 1;
}