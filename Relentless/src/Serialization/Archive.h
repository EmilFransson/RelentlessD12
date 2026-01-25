#pragma once
#include "Serialization/Archive.h"

namespace Relentless
{
	enum class EArchiveFormat : uint8 { Text = 0, Binary };

	using StreamPos = std::streampos;

	template<typename T, typename Archive>
	concept Serializable = requires(T t, Archive & ar)
	{
		{ t.Serialize(ar) } -> std::same_as<bool>;
	};

	class RLS_API IArchive
	{
	public:
		inline static std::atomic<uint32> s_ActiveCounter = 0u;

		IArchive(const Path& aPath, EArchiveFormat aFormat = EArchiveFormat::Text) noexcept;
		virtual ~IArchive() noexcept = default;

		NO_DISCARD const Path& GetSourcePath() const noexcept;

		NO_DISCARD bool IsBinaryFormat() const noexcept;
		NO_DISCARD bool IsLoading() const noexcept;
		NO_DISCARD bool IsOpen() const noexcept;
		NO_DISCARD bool IsSaving() const noexcept;
		NO_DISCARD bool IsTextFormat() const noexcept;
		NO_DISCARD bool IsValid() const noexcept;

		virtual bool Seek(StreamPos aStreamPos) noexcept = 0;

		NO_DISCARD virtual StreamPos Tell() noexcept = 0;

		NO_DISCARD virtual uint64 ProcessedBytes() const noexcept;

		template<typename... Ts>
		__forceinline bool Process(Ts&&... objects)
		{
			return (ProcessOne(std::forward<Ts>(objects)) && ...);
		}
		
		virtual bool ProcessRaw(void* aDataPtr, size_t aSize) = 0;
	protected:
		template<typename T>
		requires (std::is_fundamental_v<T> || std::is_enum_v<T>)
		__forceinline bool ProcessOne(T& aValue)
		{
			if (IsLoading())
				return ReadRaw(&aValue, sizeof(T));
			else
				return WriteRaw(&aValue, sizeof(T));
		}

		template<typename T>
		requires (std::is_fundamental_v<T> || std::is_enum_v<T>)
		__forceinline bool ProcessOne(const T& aValue)
		{
			return WriteRaw(&aValue, sizeof(T));
		}

		__forceinline bool ProcessOne(UUID& aUUID)
		{
			return 
				ProcessOne(aUUID.Data1) &&
				ProcessOne(aUUID.Data2) &&
				ProcessOne(aUUID.Data3) &&
				Process(aUUID.Data4);
		}

		__forceinline bool ProcessOne(String& aString)
		{
			uint64 length = aString.size();
			if (!ProcessOne(length))
				return false;

			if (IsLoading())
			{
				aString.resize(static_cast<size_t>(length));
				return ReadRaw(aString.data(), static_cast<size_t>(length));
			}
			else
			{
				return WriteRaw(aString.data(), static_cast<size_t>(length));
			}
		}

		__forceinline bool ProcessOne(Matrix& aMatrix)
		{
			if (IsLoading()) 
				return ReadRaw(&aMatrix, sizeof(Matrix));
			else            
				return WriteRaw(&aMatrix, sizeof(Matrix));
		}

		template<typename T>
		__forceinline bool ProcessOne(std::vector<T>& aVector)
		{
			uint64 size = static_cast<uint64>(aVector.size());
			if (!ProcessOne(size))
				return false;

			if (IsLoading())
				aVector.resize(static_cast<size_t>(size));

			for (auto& element : aVector)
			{
				if (!Process(element))
					return false;
			}

			return true;
		}

		template<typename T, size_t N>
		__forceinline bool ProcessOne(std::array<T, N>& aArray)
		{
			uint64 size = static_cast<uint64>(aArray.size());
			if (!ProcessOne(size))
				return false;

			for (auto& element : aArray)
			{
				if (!Process(element))
					return false;
			}

			return true;
		}

		template<typename Key, typename Value, typename Hash, typename KeyEqual, typename Allocator>
		__forceinline bool ProcessOne(std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>& aMap)
		{
			uint64 count = static_cast<uint64>(aMap.size());

			if (!ProcessOne(count))
				return false;

			if (IsLoading())
			{
				aMap.clear();
				aMap.reserve(static_cast<uint64>(count));

				for (uint64 i = 0; i < count; ++i)
				{
					Key   key{};
					Value val{};

					if (!Process(key))
						return false;
					if (!Process(val))
						return false;

					aMap.emplace(std::move(key), std::move(val));
				}
			}
			else
			{
				for (auto& [key, val] : aMap)
				{
					if (!Process(key))
						return false;
					if (!Process(val))
						return false;
				}
			}

			return true;
		}

		template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
		__forceinline bool ProcessOne(std::unordered_set<Key, Hash, KeyEqual, Allocator>& aSet)
		{
			uint64 count = static_cast<uint64>(aSet.size());
			if (!ProcessOne(count))
				return false;

			if (IsLoading())
			{
				aSet.clear();
				aSet.reserve(static_cast<size_t>(count));

				for (uint64 i = 0; i < count; ++i)
				{
					Key value{};
					if (!Process(value))
						return false;

					aSet.emplace(std::move(value));
				}
			}
			else
			{
				for (const Key& value : aSet)
				{
					if (!Process(value))
						return false;
				}
			}

			return true;
		}

		template<typename Key, typename Value, typename Compare, typename Allocator>
		__forceinline bool ProcessOne(std::map<Key, Value, Compare, Allocator>& aMap)
		{
			uint64 count = static_cast<uint64>(aMap.size());

			if (!ProcessOne(count))
				return false;

			if (IsLoading())
			{
				aMap.clear();

				for (uint64 i = 0; i < count; ++i)
				{
					Key   key{};
					Value val{};

					if (!Process(key))
						return false;
					if (!Process(val))
						return false;

					aMap.emplace(std::move(key), std::move(val));
				}
			}
			else
			{
				for (auto& [key, val] : aMap)
				{
					if (!Process(key))
						return false;
					if (!Process(val))
						return false;
				}
			}

			return true;
		}

		template<typename Key, typename Compare, typename Allocator>
		__forceinline bool ProcessOne(std::set<Key, Compare, Allocator>& aSet)
		{
			uint64 count = static_cast<uint64>(aSet.size());
			if (!ProcessOne(count))
				return false;

			if (IsLoading())
			{
				aSet.clear();

				for (uint64 i = 0; i < count; ++i)
				{
					Key value{};
					if (!Process(value))
						return false;

					aSet.emplace(std::move(value));
				}
			}
			else
			{
				for (const Key& value : aSet)
				{
					if (!Process(value))
						return false;
				}
			}

			return true;
		}

		template<typename T>
		requires (std::is_class_v<T> && !Serializable<T, IArchive>)
		__forceinline bool ProcessOne(T& aObject)
		{
			auto& [...fields] = aObject;
			return Process(fields...);
		}

		template<typename T>
		requires (Serializable<T, IArchive>)
		__forceinline bool ProcessOne(T& aObject)
		{
			return aObject.Serialize(*this);
		}

		template<typename T, size_t N>
		__forceinline bool ProcessOne(T(&aArray)[N])
		{
			if constexpr (std::is_trivially_copyable_v<T>)
			{
				if (IsLoading()) 
					return ReadRaw(aArray, sizeof(T) * N);
				else            
					return WriteRaw(aArray, sizeof(T) * N);
			}
			else
			{
				for (size_t i = 0; i < N; ++i)
				{
					if (!ProcessOne(aArray[i]))
						return false;
				}
				return true;
			}
		}

		virtual bool WriteRaw(const void* aDataPtr, size_t aSize) = 0;
		virtual bool ReadRaw(void* aDataPtr, size_t aSize) = 0;

	protected:
		Path m_Path;
		uint64 m_ProcessedBytes = 0u;
		EArchiveFormat m_Format = EArchiveFormat::Text;
		bool m_IsOpen = false;
		bool m_IsValid = false;
		bool m_IsSaving = true;
	};

	class RLS_API SaveArchive : public IArchive
	{
	public:
		SaveArchive(const Path& aPath, EArchiveFormat aFormat = EArchiveFormat::Text) noexcept;
		virtual ~SaveArchive() noexcept override;

		bool Flush() noexcept;

		virtual bool ProcessRaw(void* aDataPtr, size_t aSize) noexcept override;
		
		bool WriteRaw(const void* aDataPtr, size_t aSize) override
		{
			RLS_ASSERT(IsOpen() && IsValid(), "[SaveArchive::WriteRaw]: File is invalid.");

			if (!IsOpen() || !IsValid())
				return false;

			m_File.write(static_cast<const char*>(aDataPtr), aSize);
			
			m_IsValid = m_File.good();
			m_ProcessedBytes += aSize;

			return m_IsValid;
		}

		bool ReadRaw(void*, size_t) override
		{
			RLS_ASSERT(false, "ReadRaw called on SaveArchive");
			return false;
		}

		virtual bool Seek(StreamPos aStreamPos) noexcept override;
		void SetFileHiddenOnDone(bool aState) noexcept;
		
		NO_DISCARD virtual StreamPos Tell() noexcept override;
	private:
		void BuildTempPath() noexcept;
	private:
		Path m_TempPath;
		std::ofstream m_File;
		bool m_SetFileHiddenOnDone = false;
	};

	class RLS_API LoadArchive : public IArchive
	{
	public:
		LoadArchive(const Path& aPath, EArchiveFormat aFormat = EArchiveFormat::Text) noexcept;
		virtual ~LoadArchive() noexcept override;

		virtual bool ProcessRaw(void* aDataPtr, size_t aSize) noexcept override;

		bool ReadRaw(void* aDataPtr, size_t aSize) override
		{
			RLS_ASSERT(IsOpen() && IsValid(), "[LoadArchive::ReadRaw]: File is invalid.");

			if (!IsOpen() || !IsValid())
				return false;

			m_File.read(static_cast<char*>(aDataPtr), aSize);
			m_IsValid = m_File.good();
			m_ProcessedBytes += aSize;

			return m_IsValid;
		}

		bool WriteRaw(const void*, size_t) override
		{
			RLS_ASSERT(false, "WriteRaw called on LoadArchive");
			return false;
		}
		
		virtual bool Seek(StreamPos aStreamPos) noexcept override;

		NO_DISCARD virtual StreamPos Tell() noexcept override;
	private:
		std::ifstream m_File;
	};
}