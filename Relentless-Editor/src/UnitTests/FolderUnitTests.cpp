#include "FolderUnitTests.h"
#include "../Core/EntityFolders.h"

#include <Relentless.h>

namespace Relentless
{
	namespace EntityFoldersUnitTests
	{
		static bool StartsWith(const String& s, std::string_view prefix)
		{
			return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
		}

		static bool ConsoleIsUTF8()
		{
			return GetConsoleOutputCP() == 65001; // CP_UTF8
		}
		struct TreeGlyphs { const char* elbow; const char* tee; const char* pipe; const char* space; };

		void Execute()
		{
			Scene scene("Folder Unit Test Scene");
			EntityFoldersManager foldersManager(nullptr); //No need for editor support in unit tests

			const FolderRoot root = FolderRoot::CreateFromScene(scene);

			auto dump = [&](std::string_view tag)
				{
					const TreeGlyphs g = ConsoleIsUTF8()
						? TreeGlyphs{ "└─ ", "├─ ", "│  ", "   " }   // UTF-8 bytes in source
					: TreeGlyphs{ "\\- ", "|- ", "|  ", "   " }; // ASCII fallback

					std::cout << "\n=== " << tag << " ===\n";

					// Gather folders
					auto& cont = foldersManager.GetFolderContainer(scene).Folders;
					std::vector<EntityFolder*> nodes; nodes.reserve(cont.size());
					for (auto& rf : cont) nodes.push_back(rf);

					// Build children map by parent*
					std::unordered_map<EntityFolder*, std::vector<EntityFolder*>> children;
					for (auto* n : nodes) children[n->GetParent()].push_back(n);
					for (auto& [p, ch] : children)
						std::sort(ch.begin(), ch.end(),
							[](auto* a, auto* b) { return a->GetLabel() < b->GetLabel(); });

					// Tree print
					std::function<void(EntityFolder*, std::string, bool)> dfs =
						[&](EntityFolder* n, std::string indent, bool last)
						{
							std::cout << indent << (last ? g.elbow : g.tee) << n->GetPath() << "\n";
							auto it = children.find(n);
							if (it == children.end()) return;
							auto& ch = it->second;
							for (size_t i = 0; i < ch.size(); ++i)
								dfs(ch[i], indent + (last ? g.space : g.pipe), i + 1 == ch.size());
						};

					std::cout << "[Folders :: tree]\n";
					if (auto it = children.find(nullptr); it != children.end()) {
						auto& roots = it->second;
						for (size_t i = 0; i < roots.size(); ++i)
							dfs(roots[i], "", i + 1 == roots.size());
					}
					else {
						std::cout << "(no folders)\n";
					}

					// Flat list + duplicates
					std::vector<EntityFolder*> sorted = nodes;
					std::sort(sorted.begin(), sorted.end(),
						[](auto* a, auto* b) { return a->GetPath() < b->GetPath(); });

					std::unordered_map<String, int> pathCount;
					for (auto* n : sorted) ++pathCount[n->GetPath()];

					std::cout << "\n[Folders :: flat]\n";
					for (auto* n : sorted) {
						const auto* p = n->GetParent();
						const bool dup = pathCount[n->GetPath()] > 1;
						std::cout << " - " << n->GetPath()
							<< "   (label=\"" << n->GetLabel() << "\""
							<< ", parent=" << (p ? p->GetPath() : "<null>")
							<< (dup ? ", DUPLICATE" : "") << ")\n";
					}

					// Entities
					struct Row { entity e; String name; String path; };
					std::vector<Row> rows;
					scene.GetEntityManager().Collect<FolderComponent>().Do(
						[&](entity e, const FolderComponent& fc)
						{
							rows.push_back({ e,
											 scene.GetEntityManager().Get<NameComponent>(e).Name,
											 fc.Folder.GetPath() });
						});
					std::sort(rows.begin(), rows.end(),
						[](const Row& a, const Row& b) {
							if (a.path != b.path) return a.path < b.path;
							return a.name < b.name;
						});

					std::cout << "\n[Entities]\n";
					for (const auto& r : rows)
						std::cout << " - " << r.path << "   " << r.name
						<< " (e=" << (uint64_t)r.e << ")\n";
				};

			// Clean start (optional): ensure no pre-existing conflicting folders/entities
			// You can clear your scene here if you have a helper.

			foldersManager.CreateFolder(scene, "Root/A/B");
			foldersManager.CreateFolder(scene, "Root/B/A/A");
			entity e = scene.CreateEntity("X");
			foldersManager.AttachEntityToFolder(scene, e, Folder(root, "Root/B/A/A"));

			dump("before delete Root/B");
			foldersManager.DeleteFolder(scene, "Root/B");
			dump("after delete Root/B");

			RLS_ASSERT(scene.GetEntityManager().Get<FolderComponent>(e).Folder.GetPath() == "Root/A/A", "Should be merged under Root/A/A");

			// ─────────────────────────────────────────────────────────────────────────────
			// 1) Create chain and idempotency
			// ─────────────────────────────────────────────────────────────────────────────
			{
				EntityFolder* a = foldersManager.CreateFolder(scene, "A");
				RLS_ASSERT(a && a->GetLabel() == "A", "Create A failed");

				EntityFolder* ab = foldersManager.CreateFolder(scene, "A/B");
				RLS_ASSERT(ab && ab->GetLabel() == "B" && ab->GetParent() == a, "Create A/B failed");

				// Idempotent create: returns the same object and keeps correct parent
				EntityFolder* ab2 = foldersManager.CreateFolder(scene, "A/B");
				RLS_ASSERT(ab2 == ab, "Idempotent CreateFolder returned a different object");
				RLS_ASSERT(ab2->GetParent() == a, "Idempotent CreateFolder changed parent incorrectly");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 2) Default name uniqueness
			// ─────────────────────────────────────────────────────────────────────────────
			{
				String p = foldersManager.GetDefaultFolderName(scene, "A");
				RLS_ASSERT(p == "A/New Folder", "Expected A/New Folder");

				foldersManager.CreateFolder(scene, p); // create it

				String p2 = foldersManager.GetDefaultFolderName(scene, "A");
				RLS_ASSERT(p2 == "A/New Folder2", "Expected A/New Folder2");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 3) Boundary correctness: "Foo" vs "FooBar"
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersManager.CreateFolder(scene, "Foo");
				foldersManager.CreateFolder(scene, "FooBar"); // must not be affected by renaming Foo

				const entity eFoo = scene.CreateEntity("InFoo");
				const entity eFooBar = scene.CreateEntity("InFooBar");

				foldersManager.AttachEntityToFolder(scene, eFoo, Folder(root, "Foo"));
				foldersManager.AttachEntityToFolder(scene, eFooBar, Folder(root, "FooBar"));

				// Rename Foo -> Baz
				bool ok = foldersManager.RenameFolder(scene, "Foo", "Baz");
				RLS_ASSERT(ok, "Rename Foo->Baz failed");

				const auto& pFoo = scene.GetEntityManager().Get<FolderComponent>(eFoo).Folder.GetPath();
				const auto& pFooB = scene.GetEntityManager().Get<FolderComponent>(eFooBar).Folder.GetPath();
				RLS_ASSERT(pFoo == "Baz", "Entity in Foo not remapped to Baz");
				RLS_ASSERT(pFooB == "FooBar", "Entity in FooBar should not be affected");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 4) Deep subtree rename + entity remap
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersManager.CreateFolder(scene, "StarterContent/Entities/Monsters");
				foldersManager.CreateFolder(scene, "StarterContent/Entities/Monsters/Bosses");
				foldersManager.CreateFolder(scene, "StarterContent/Props");

				const entity ground = scene.CreateEntity("Ground");
				const entity troll = scene.CreateEntity("TrollBoss");
				const entity crate = scene.CreateEntity("Crate");

				foldersManager.AttachEntityToFolder(scene, ground, Folder(root, "StarterContent/Entities/Monsters"));
				foldersManager.AttachEntityToFolder(scene, troll, Folder(root, "StarterContent/Entities/Monsters/Bosses"));
				foldersManager.AttachEntityToFolder(scene, crate, Folder(root, "StarterContent/Props"));

				dump("before deep rename");

				// Monsters -> Enemies
				bool ok = foldersManager.RenameFolder(scene,
					"StarterContent/Entities/Monsters",
					"StarterContent/Entities/Enemies");
				RLS_ASSERT(ok, "Rename Monsters->Enemies failed");

				dump("after deep rename");

				const auto& groundPath = scene.GetEntityManager().Get<FolderComponent>(ground).Folder.GetPath();
				const auto& trollPath = scene.GetEntityManager().Get<FolderComponent>(troll).Folder.GetPath();
				const auto& cratePath = scene.GetEntityManager().Get<FolderComponent>(crate).Folder.GetPath();

				RLS_ASSERT(groundPath == "StarterContent/Entities/Enemies", "Remap failed for ground");
				RLS_ASSERT(trollPath == "StarterContent/Entities/Enemies/Bosses", "Remap failed for troll");
				RLS_ASSERT(cratePath == "StarterContent/Props", "Unrelated entity changed unexpectedly");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 5) Invalid rename: into own descendant
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersManager.CreateFolder(scene, "X/Y");
				foldersManager.CreateFolder(scene, "X/Y/Z");

				bool ok = foldersManager.RenameFolder(scene, "X/Y", "X/Y/Z"); // invalid
				RLS_ASSERT(!ok, "Should not allow rename into own descendant");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 6) Rename to an existing path (should fail, no changes)
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersManager.CreateFolder(scene, "Alpha");
				foldersManager.CreateFolder(scene, "Beta");

				// Put an entity in Alpha to detect unintended changes
				const entity a = scene.CreateEntity("A");
				foldersManager.AttachEntityToFolder(scene, a, Folder(root, "Alpha"));

				bool ok = foldersManager.RenameFolder(scene, "Alpha", "Beta");
				RLS_ASSERT(!ok, "Rename to existing folder should fail");

				const auto& pathA = scene.GetEntityManager().Get<FolderComponent>(a).Folder.GetPath();
				RLS_ASSERT(pathA == "Alpha", "Entity path changed despite failed rename");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 7) Delete middle parent: children re-parent and entities remap away from prefix
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersManager.CreateFolder(scene, "SC/Entities/Enemies/Minions");
				const entity m1 = scene.CreateEntity("Minion1");
				const entity m2 = scene.CreateEntity("Minion2");

				foldersManager.AttachEntityToFolder(scene, m1, Folder(root, "SC/Entities/Enemies"));
				foldersManager.AttachEntityToFolder(scene, m2, Folder(root, "SC/Entities/Enemies/Minions"));

				dump("before delete SC/Entities");

				// Delete middle parent "SC/Entities"
				foldersManager.DeleteFolder(scene, "SC/Entities");

				dump("after delete SC/Entities");

				const auto& mp1 = scene.GetEntityManager().Get<FolderComponent>(m1).Folder.GetPath();
				const auto& mp2 = scene.GetEntityManager().Get<FolderComponent>(m2).Folder.GetPath();

				// Policy-agnostic check: old prefix must be gone
				RLS_ASSERT(!StartsWith(mp1, "SC/Entities/"), "m1 still has deleted prefix");
				RLS_ASSERT(!StartsWith(mp2, "SC/Entities/"), "m2 still has deleted prefix");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 8) ForEachEntityInFolders: selection + early break
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersManager.CreateFolder(scene, "Pick/One");
				foldersManager.CreateFolder(scene, "Pick/Two");

				const entity e1 = scene.CreateEntity("Pick1");
				const entity e2 = scene.CreateEntity("Pick2");
				const entity e3 = scene.CreateEntity("Pick3");

				foldersManager.AttachEntityToFolder(scene, e1, Folder(root, "Pick/One"));
				foldersManager.AttachEntityToFolder(scene, e2, Folder(root, "Pick/Two"));
				foldersManager.AttachEntityToFolder(scene, e3, Folder(root, "Pick/Two"));

				std::unordered_set<String> paths = { "Pick/One", "Pick/Two" };

				std::vector<entity> visited;
				foldersManager.ForEachEntityInFolders(scene, paths,
					[&](entity e)
					{
						visited.push_back(e);
						// Early break after 2
						return visited.size() < 2;
					});

				RLS_ASSERT(visited.size() == 2, "Early-break in ForEachEntityInFolders failed");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 9) Expanded state flag round-trip
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersManager.CreateFolder(scene, "UI/Windows");
				RLS_ASSERT(foldersManager.IsFolderExpanded(scene, "UI/Windows"), "Expected collapsed by default");

				if (Ref<EntityFolder> f = foldersManager.GetFolder(scene, "UI/Windows"))
					f->SetExpandedState(false);

				RLS_ASSERT(!foldersManager.IsFolderExpanded(scene, "UI/Windows"), "Expanded state not persisted");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 10) Root-level rename (no parent)
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersManager.CreateFolder(scene, "Top");
				const entity t = scene.CreateEntity("TopGuy");
				foldersManager.AttachEntityToFolder(scene, t, Folder(root, "Top"));

				bool ok = foldersManager.RenameFolder(scene, "Top", "TopRenamed");
				RLS_ASSERT(ok, "Root-level rename failed");

				const auto& tp = scene.GetEntityManager().Get<FolderComponent>(t).Folder.GetPath();
				RLS_ASSERT(tp == "TopRenamed", "Root-level entity not remapped");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 11) Delete leaf, no crash, entity policy respected (no old prefix)
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersManager.CreateFolder(scene, "Leaf");
				const entity l = scene.CreateEntity("Leafy");
				foldersManager.AttachEntityToFolder(scene, l, Folder(root, "Leaf"));

				foldersManager.DeleteFolder(scene, "Leaf");
				bool has = scene.GetEntityManager().Has<FolderComponent>(l);
				RLS_ASSERT(!has, "Leaf entity still references deleted folder");
			}

			std::cout << "\nAll folder tests passed ✅\n";
		}
	}
}
