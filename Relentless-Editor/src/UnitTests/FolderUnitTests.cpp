#include "FolderUnitTests.h"
#include <Relentless.h>

#include "../Subsystem/EntityFoldersSubsystem.h"

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
			EntityFoldersSubsystem foldersSubsystem; //No need for editor support in unit tests

			const FolderRoot root = FolderRoot::CreateFromScene(scene);

			auto dump = [&](std::string_view tag)
				{
					const TreeGlyphs g = ConsoleIsUTF8()
						? TreeGlyphs{ "└─ ", "├─ ", "│  ", "   " }   // UTF-8 bytes in source
					: TreeGlyphs{ "\\- ", "|- ", "|  ", "   " }; // ASCII fallback

					std::cout << "\n=== " << tag << " ===\n";

					// Gather folders
					auto& cont = foldersSubsystem.GetFolderContainer(scene).Folders;
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
											 scene.GetEntityManager().Get<NameComponent>(e).GetName(),
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

			foldersSubsystem.CreateFolder(scene, "Root/A/B");
			foldersSubsystem.CreateFolder(scene, "Root/B/A/A");
			entity e = scene.CreateEntity("X");
			foldersSubsystem.AttachEntityToFolder(scene, e, Folder(root, "Root/B/A/A"));

			dump("before delete Root/B");
			foldersSubsystem.DeleteFolder(scene, "Root/B");
			dump("after delete Root/B");

			RLS_VERIFY(scene.GetEntityManager().Get<FolderComponent>(e).Folder.GetPath() == "Root/A/A", "Should be merged under Root/A/A");

			// ─────────────────────────────────────────────────────────────────────────────
			// 1) Create chain and idempotency
			// ─────────────────────────────────────────────────────────────────────────────
			{
				EntityFolder* a = foldersSubsystem.CreateFolder(scene, "A");
				RLS_VERIFY(a && a->GetLabel() == "A", "Create A failed");

				EntityFolder* ab = foldersSubsystem.CreateFolder(scene, "A/B");
				RLS_VERIFY(ab && ab->GetLabel() == "B" && ab->GetParent() == a, "Create A/B failed");

				// Idempotent create: returns the same object and keeps correct parent
				EntityFolder* ab2 = foldersSubsystem.CreateFolder(scene, "A/B");
				RLS_VERIFY(ab2 == ab, "Idempotent CreateFolder returned a different object");
				RLS_VERIFY(ab2->GetParent() == a, "Idempotent CreateFolder changed parent incorrectly");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 2) Default name uniqueness
			// ─────────────────────────────────────────────────────────────────────────────
			{
				String p = foldersSubsystem.GetDefaultFolderName(scene, "A");
				RLS_VERIFY(p == "A/New Folder", "Expected A/New Folder");

				foldersSubsystem.CreateFolder(scene, p); // create it

				String p2 = foldersSubsystem.GetDefaultFolderName(scene, "A");
				RLS_VERIFY(p2 == "A/New Folder2", "Expected A/New Folder2");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 3) Boundary correctness: "Foo" vs "FooBar"
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersSubsystem.CreateFolder(scene, "Foo");
				foldersSubsystem.CreateFolder(scene, "FooBar"); // must not be affected by renaming Foo

				const entity eFoo = scene.CreateEntity("InFoo");
				const entity eFooBar = scene.CreateEntity("InFooBar");

				foldersSubsystem.AttachEntityToFolder(scene, eFoo, Folder(root, "Foo"));
				foldersSubsystem.AttachEntityToFolder(scene, eFooBar, Folder(root, "FooBar"));

				// Rename Foo -> Baz
				bool ok = foldersSubsystem.RenameFolder(scene, "Foo", "Baz");
				RLS_VERIFY(ok, "Rename Foo->Baz failed");

				const auto& pFoo = scene.GetEntityManager().Get<FolderComponent>(eFoo).Folder.GetPath();
				const auto& pFooB = scene.GetEntityManager().Get<FolderComponent>(eFooBar).Folder.GetPath();
				RLS_VERIFY(pFoo == "Baz", "Entity in Foo not remapped to Baz");
				RLS_VERIFY(pFooB == "FooBar", "Entity in FooBar should not be affected");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 4) Deep subtree rename + entity remap
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersSubsystem.CreateFolder(scene, "StarterContent/Entities/Monsters");
				foldersSubsystem.CreateFolder(scene, "StarterContent/Entities/Monsters/Bosses");
				foldersSubsystem.CreateFolder(scene, "StarterContent/Props");

				const entity ground = scene.CreateEntity("Ground");
				const entity troll = scene.CreateEntity("TrollBoss");
				const entity crate = scene.CreateEntity("Crate");

				foldersSubsystem.AttachEntityToFolder(scene, ground, Folder(root, "StarterContent/Entities/Monsters"));
				foldersSubsystem.AttachEntityToFolder(scene, troll, Folder(root, "StarterContent/Entities/Monsters/Bosses"));
				foldersSubsystem.AttachEntityToFolder(scene, crate, Folder(root, "StarterContent/Props"));

				dump("before deep rename");

				// Monsters -> Enemies
				bool ok = foldersSubsystem.RenameFolder(scene,
					"StarterContent/Entities/Monsters",
					"StarterContent/Entities/Enemies");
				RLS_VERIFY(ok, "Rename Monsters->Enemies failed");

				dump("after deep rename");

				const auto& groundPath = scene.GetEntityManager().Get<FolderComponent>(ground).Folder.GetPath();
				const auto& trollPath = scene.GetEntityManager().Get<FolderComponent>(troll).Folder.GetPath();
				const auto& cratePath = scene.GetEntityManager().Get<FolderComponent>(crate).Folder.GetPath();

				RLS_VERIFY(groundPath == "StarterContent/Entities/Enemies", "Remap failed for ground");
				RLS_VERIFY(trollPath == "StarterContent/Entities/Enemies/Bosses", "Remap failed for troll");
				RLS_VERIFY(cratePath == "StarterContent/Props", "Unrelated entity changed unexpectedly");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 5) Invalid rename: into own descendant
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersSubsystem.CreateFolder(scene, "X/Y");
				foldersSubsystem.CreateFolder(scene, "X/Y/Z");

				bool ok = foldersSubsystem.RenameFolder(scene, "X/Y", "X/Y/Z"); // invalid
				RLS_VERIFY(!ok, "Should not allow rename into own descendant");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 6) Rename to an existing path (should fail, no changes)
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersSubsystem.CreateFolder(scene, "Alpha");
				foldersSubsystem.CreateFolder(scene, "Beta");

				// Put an entity in Alpha to detect unintended changes
				const entity a = scene.CreateEntity("A");
				foldersSubsystem.AttachEntityToFolder(scene, a, Folder(root, "Alpha"));

				bool ok = foldersSubsystem.RenameFolder(scene, "Alpha", "Beta");
				RLS_VERIFY(!ok, "Rename to existing folder should fail");

				const auto& pathA = scene.GetEntityManager().Get<FolderComponent>(a).Folder.GetPath();
				RLS_VERIFY(pathA == "Alpha", "Entity path changed despite failed rename");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 7) Delete middle parent: children re-parent and entities remap away from prefix
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersSubsystem.CreateFolder(scene, "SC/Entities/Enemies/Minions");
				const entity m1 = scene.CreateEntity("Minion1");
				const entity m2 = scene.CreateEntity("Minion2");

				foldersSubsystem.AttachEntityToFolder(scene, m1, Folder(root, "SC/Entities/Enemies"));
				foldersSubsystem.AttachEntityToFolder(scene, m2, Folder(root, "SC/Entities/Enemies/Minions"));

				dump("before delete SC/Entities");

				// Delete middle parent "SC/Entities"
				foldersSubsystem.DeleteFolder(scene, "SC/Entities");

				dump("after delete SC/Entities");

				const auto& mp1 = scene.GetEntityManager().Get<FolderComponent>(m1).Folder.GetPath();
				const auto& mp2 = scene.GetEntityManager().Get<FolderComponent>(m2).Folder.GetPath();

				// Policy-agnostic check: old prefix must be gone
				RLS_VERIFY(!StartsWith(mp1, "SC/Entities/"), "m1 still has deleted prefix");
				RLS_VERIFY(!StartsWith(mp2, "SC/Entities/"), "m2 still has deleted prefix");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 8) ForEachEntityInFolders: selection + early break
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersSubsystem.CreateFolder(scene, "Pick/One");
				foldersSubsystem.CreateFolder(scene, "Pick/Two");

				const entity e1 = scene.CreateEntity("Pick1");
				const entity e2 = scene.CreateEntity("Pick2");
				const entity e3 = scene.CreateEntity("Pick3");

				foldersSubsystem.AttachEntityToFolder(scene, e1, Folder(root, "Pick/One"));
				foldersSubsystem.AttachEntityToFolder(scene, e2, Folder(root, "Pick/Two"));
				foldersSubsystem.AttachEntityToFolder(scene, e3, Folder(root, "Pick/Two"));

				std::unordered_set<String> paths = { "Pick/One", "Pick/Two" };

				std::vector<entity> visited;
				foldersSubsystem.ForEachEntityInFolders(scene, paths,
					[&](entity e)
					{
						visited.push_back(e);
						// Early break after 2
						return visited.size() < 2;
					});

				RLS_VERIFY(visited.size() == 2, "Early-break in ForEachEntityInFolders failed");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 9) Expanded state flag round-trip
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersSubsystem.CreateFolder(scene, "UI/Windows");
				RLS_VERIFY(foldersSubsystem.IsFolderExpanded(scene, "UI/Windows"), "Expected collapsed by default");

				if (Ref<EntityFolder> f = foldersSubsystem.GetFolder(scene, "UI/Windows"))
					f->SetExpandedState(false);

				RLS_VERIFY(!foldersSubsystem.IsFolderExpanded(scene, "UI/Windows"), "Expanded state not persisted");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 10) Root-level rename (no parent)
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersSubsystem.CreateFolder(scene, "Top");
				const entity t = scene.CreateEntity("TopGuy");
				foldersSubsystem.AttachEntityToFolder(scene, t, Folder(root, "Top"));

				bool ok = foldersSubsystem.RenameFolder(scene, "Top", "TopRenamed");
				RLS_VERIFY(ok, "Root-level rename failed");

				const auto& tp = scene.GetEntityManager().Get<FolderComponent>(t).Folder.GetPath();
				RLS_VERIFY(tp == "TopRenamed", "Root-level entity not remapped");
			}

			// ─────────────────────────────────────────────────────────────────────────────
			// 11) Delete leaf, no crash, entity policy respected (no old prefix)
			// ─────────────────────────────────────────────────────────────────────────────
			{
				foldersSubsystem.CreateFolder(scene, "Leaf");
				const entity l = scene.CreateEntity("Leafy");
				foldersSubsystem.AttachEntityToFolder(scene, l, Folder(root, "Leaf"));

				foldersSubsystem.DeleteFolder(scene, "Leaf");
				bool has = scene.GetEntityManager().Has<FolderComponent>(l);
				RLS_VERIFY(!has, "Leaf entity still references deleted folder");
			}

			std::cout << "\nAll folder tests passed ✅\n";
		}
	}
}
