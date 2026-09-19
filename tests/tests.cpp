#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "../include/LavaEngine/Application.hpp"

using namespace LavaEngine;

namespace
{
    struct TestContext
    {
        int checks = 0;
        int failures = 0;
    };

    TestContext* g_currentTest = nullptr;

#define CHECK(cond)                                                        \
    do                                                                     \
    {                                                                      \
        ++g_currentTest->checks;                                           \
        if (!(cond))                                                       \
        {                                                                  \
            ++g_currentTest->failures;                                     \
            std::fprintf(                                                  \
                stderr,                                                    \
                "      FAIL %s:%d: %s\n",                                  \
                __FILE__,                                                   \
                __LINE__,                                                   \
                #cond                                                       \
            );                                                             \
        }                                                                  \
    } while (0)

#define CHECK_EQ(a, b)                                                     \
    do                                                                     \
    {                                                                      \
        ++g_currentTest->checks;                                           \
        const auto& _a = (a);                                              \
        const auto& _b = (b);                                              \
        if (!(_a == _b))                                                   \
        {                                                                  \
            ++g_currentTest->failures;                                     \
            std::fprintf(                                                  \
                stderr,                                                    \
                "      FAIL %s:%d: %s == %s\n",                            \
                __FILE__,                                                   \
                __LINE__,                                                   \
                #a,                                                         \
                #b                                                          \
            );                                                             \
        }                                                                  \
    } while (0)

#define CHECK_NE(a, b)                                                     \
    do                                                                     \
    {                                                                      \
        ++g_currentTest->checks;                                           \
        const auto& _a = (a);                                              \
        const auto& _b = (b);                                              \
        if (!(_a != _b))                                                   \
        {                                                                  \
            ++g_currentTest->failures;                                     \
            std::fprintf(                                                  \
                stderr,                                                    \
                "      FAIL %s:%d: %s != %s\n",                            \
                __FILE__,                                                   \
                __LINE__,                                                   \
                #a,                                                         \
                #b                                                          \
            );                                                             \
        }                                                                  \
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

        if (it == RecorderModule::g_order.end())
            return static_cast<std::size_t>(-1);

        return static_cast<std::size_t>(
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
        CHECK(h2.valid());
        CHECK_NE(h1, h2);
        CHECK(reg.contains(h1));
        CHECK(reg.contains(h2));
        CHECK_EQ(reg.size(), std::size_t{2});

        reg.remove(h1);

        CHECK(!reg.contains(h1));
        CHECK(reg.get<Mesh>(h1) == nullptr);
        CHECK(reg.contains(h2));
        CHECK_EQ(reg.size(), std::size_t{1});

        // Recreating at the same slot must not revive the stale handle.
        ResourceHandle h3 = reg.create<Mesh>();

        CHECK(h3.valid());
        CHECK_NE(h3, h1);
        CHECK(!reg.contains(h1));
        CHECK(reg.get<Mesh>(h1) == nullptr);
        CHECK(reg.contains(h3));
        CHECK(reg.get<Mesh>(h3) != nullptr);
        CHECK_EQ(reg.size(), std::size_t{2});

        ResourceHandle invalid;

        CHECK(!invalid.valid());
        CHECK(!reg.contains(invalid));
        CHECK(reg.get<Mesh>(invalid) == nullptr);

        reg.clear();

        CHECK(reg.empty());
        CHECK_EQ(reg.size(), std::size_t{0});
        CHECK(reg.get<Mesh>(h2) == nullptr);
        CHECK(reg.get<Mesh>(h3) == nullptr);
    }

    void testResourceViewLifetime()
    {
        ResourceView<Mesh> view;

        {
            ResourceRegistry reg;
            ResourceHandle h = reg.create<Mesh>();

            view = reg.view<Mesh>(h);

            CHECK(view.valid());
            CHECK(view.get() != nullptr);
            CHECK(static_cast<bool>(view));
            CHECK(!view.expired());

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

        CHECK(view.valid());
        CHECK(view.get() != nullptr);

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

        ResourceHandle h1 = reg.create<Mesh>();
        ResourceHandle h2 = reg.create<Mesh>();

        std::size_t count = 0;

        for (const auto& [id, entry] : reg.all())
        {
            ++count;

            CHECK(
                id == h1.id() ||
                id == h2.id()
            );

            CHECK(entry.resource != nullptr);

            if (id == h1.id())
                CHECK_EQ(entry.generation, h1.generation());

            if (id == h2.id())
                CHECK_EQ(entry.generation, h2.generation());
        }

        CHECK_EQ(count, std::size_t{2});
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
        CHECK(!view.expired());

        // A non-exported resource cannot be imported.
        Container other("other");
        ResourceHandle h2 = other.createResource<Mesh>();

        ResourceView<Mesh> view2 =
            importer.importResource<Mesh>(other, h2);

        CHECK(!view2.valid());
        CHECK(view2.get() == nullptr);

        // Destroying the exporter must invalidate imported views.
        {
            Container dying("dying");
            ResourceHandle hd = dying.createResource<Mesh>();

            dying.exportResource<Mesh>(hd);

            view = importer.importResource<Mesh>(dying, hd);

            CHECK(view.valid());
            CHECK(view.get() != nullptr);
        }

        CHECK(!view.valid());
        CHECK(view.get() == nullptr);
        CHECK(view.expired());
    }

    void testContainerMoveRepointsModules()
    {
        Container a("a");

        RecorderModule& module =
            a.addModule<RecorderModule>();

        CHECK(&module.getContainer() == &a);
        CHECK(a.hasModule<RecorderModule>());

        Container b = std::move(a);

        CHECK(&module.getContainer() == &b);
        CHECK(b.hasModule<RecorderModule>());
        CHECK(b.getModule<RecorderModule>() == &module);

        Container c("c");

        c = std::move(b);

        CHECK(&module.getContainer() == &c);
        CHECK(c.hasModule<RecorderModule>());
        CHECK(c.getModule<RecorderModule>() == &module);
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

        CHECK_EQ(c.variables().size(), std::size_t{5});

        Variable* flag = nullptr;
        Variable* count = nullptr;
        Variable* speed = nullptr;
        Variable* scale = nullptr;
        Variable* title = nullptr;

        for (Variable& variable : c.variables())
        {
            if (variable.name == "flag")
                flag = &variable;
            else if (variable.name == "count")
                count = &variable;
            else if (variable.name == "speed")
                speed = &variable;
            else if (variable.name == "scale")
                scale = &variable;
            else if (variable.name == "title")
                title = &variable;
        }

        CHECK(flag != nullptr);
        CHECK(count != nullptr);
        CHECK(speed != nullptr);
        CHECK(scale != nullptr);
        CHECK(title != nullptr);

        // Simulate Inspector edits.
        *static_cast<bool*>(flag->ptr) = false;
        *static_cast<int*>(count->ptr) = 42;
        *static_cast<float*>(speed->ptr) = 99.0f;
        *static_cast<double*>(scale->ptr) = 123.0;
        *static_cast<std::string*>(title->ptr) = "edited";

        // Exposed variables are boxed copies, not aliases to caller memory.
        CHECK(callerBool);
        CHECK_EQ(callerInt, 7);
        CHECK_EQ(callerFloat, 1.5f);
        CHECK_EQ(callerDouble, 2.5);
        CHECK_EQ(callerString, std::string{"original"});

        CHECK(!*static_cast<bool*>(flag->ptr));
        CHECK_EQ(*static_cast<int*>(count->ptr), 42);
        CHECK_EQ(*static_cast<float*>(speed->ptr), 99.0f);
        CHECK_EQ(*static_cast<double*>(scale->ptr), 123.0);
        CHECK_EQ(
            *static_cast<std::string*>(title->ptr),
            std::string{"edited"}
        );
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

        const std::size_t containerHook =
            positionOf("container.onUnload");

        const std::size_t moduleHook =
            positionOf("module.onUnload");

        const std::size_t moduleDtor =
            positionOf("module.dtor");

        const std::size_t containerDtor =
            positionOf("container.dtor");

        CHECK_EQ(containerHook, std::size_t{0});

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

        CHECK(
            RecorderModule::g_order ==
            std::vector<std::string>{"module.onUnload"}
        );
    }

    struct IdTypeA : Module
    {
        void imgui() const override {}
    };

    struct IdTypeB : Module
    {
        void imgui() const override {}
    };

    void testTypeIdStability()
    {
        // Compile-time stable and deterministic: the same type always maps
        // to the same ID, and distinct types map to distinct IDs.
        static_assert(typeID<IdTypeA>() == typeID<IdTypeA>());
        static_assert(typeID<IdTypeB>() == typeID<IdTypeB>());
        static_assert(typeID<int>() == typeID<int>());

        CHECK_EQ(typeID<IdTypeA>(), typeID<IdTypeA>());
        CHECK_EQ(typeID<IdTypeB>(), typeID<IdTypeB>());
        CHECK_NE(typeID<IdTypeA>(), typeID<IdTypeB>());

        // The ID derives from the type's name, not from a typeid token, so
        // a hot-reloaded copy of the same source type still maps to the same
        // key. Building the registry with the name-derived ID feels like any
        // clean compile: no collision errors, adds/removes resolve.
        Container c("c");

        IdTypeA& a = c.addModule<IdTypeA>();

        CHECK(c.hasModule<IdTypeA>());
        CHECK(c.getModule<IdTypeA>() == &a);

        c.removeModule<IdTypeA>();

        CHECK(!c.hasModule<IdTypeA>());
    }

    struct TestCase
    {
        const char* name;
        void (*function)();
    };

    constexpr TestCase tests[] =
    {
        {"Generation handles", testGenerationHandles},
        {"Resource view lifetime", testResourceViewLifetime},
        {"Views survive registry moves", testViewSurvivesRegistryMove},
        {"Resource iteration", testAllIteration},
        {"Container resource import", testContainerImportResource},
        {"Container module move", testContainerMoveRepointsModules},
        {"Exposed variables", testExposedBoxes},
        {"Teardown ordering", testTeardownHooksRunBeforeDestruction},
        {"Default container unload", testDefaultContainerOnUnload},
        {"Type ID stability", testTypeIdStability},
    };

    void printSeparator()
    {
        std::printf(
            "------------------------------------------------------------\n"
        );
    }
}

int main()
{
    const auto suiteStart =
        std::chrono::steady_clock::now();

    int passed = 0;
    int failed = 0;
    int totalChecks = 0;
    int totalFailures = 0;

    std::printf(
        "\n"
        "LavaEngine Unit Tests\n"
    );

    printSeparator();

    for (const TestCase& test : tests)
    {
        TestContext context;
        g_currentTest = &context;

        const auto start =
            std::chrono::steady_clock::now();

        bool threw = false;

        try
        {
            test.function();
        }
        catch (const std::exception& e)
        {
            threw = true;
            ++context.failures;

            std::fprintf(
                stderr,
                "      EXCEPTION: %s\n",
                e.what()
            );
        }
        catch (...)
        {
            threw = true;
            ++context.failures;

            std::fprintf(
                stderr,
                "      EXCEPTION: unknown exception\n"
            );
        }

        const auto end =
            std::chrono::steady_clock::now();

        const double milliseconds =
            std::chrono::duration<double, std::milli>(
                end - start
            ).count();

        totalChecks += context.checks;
        totalFailures += context.failures;

        if (context.failures == 0 && !threw)
        {
            ++passed;

            std::printf(
                "  [PASS] %-32s %4d checks  %.3f ms\n",
                test.name,
                context.checks,
                milliseconds
            );
        }
        else
        {
            ++failed;

            std::printf(
                "  [FAIL] %-32s %4d checks  %.3f ms\n",
                test.name,
                context.checks,
                milliseconds
            );
        }
    }

    g_currentTest = nullptr;

    const auto suiteEnd =
        std::chrono::steady_clock::now();

    const double totalMilliseconds =
        std::chrono::duration<double, std::milli>(
            suiteEnd - suiteStart
        ).count();

    printSeparator();

    std::printf(
        "Tests:   %d passed, %d failed, %zu total\n",
        passed,
        failed,
        sizeof(tests) / sizeof(tests[0])
    );

    std::printf(
        "Checks:  %d executed, %d failed\n",
        totalChecks,
        totalFailures
    );

    std::printf(
        "Time:    %.3f ms\n",
        totalMilliseconds
    );

    if (failed == 0)
    {
        std::printf(
            "\n"
            "RESULT: PASS\n"
        );

        return 0;
    }

    std::printf(
        "\n"
        "RESULT: FAIL\n"
    );

    return 1;
}
