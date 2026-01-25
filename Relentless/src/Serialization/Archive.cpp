#include "Archive.h"
#include "File/File.h"
#include "Process/Process.h"
#include "Utility/FilepathUtils.h"
#include "Utility/StringUtils.h"

namespace Relentless
{
	IArchive::IArchive(const Path& aPath, EArchiveFormat aFormat) noexcept
		: m_Path{aPath}
		, m_Format{aFormat}
	{
	}

	const Path& IArchive::GetSourcePath() const noexcept
	{
		return m_Path;
	}

	bool IArchive::IsBinaryFormat() const noexcept
	{
		return m_Format == EArchiveFormat::Binary;
	}

	bool IArchive::IsLoading() const noexcept
	{
		return !m_IsSaving;
	}

	bool IArchive::IsOpen() const noexcept
	{
		return m_IsOpen;
	}

	bool IArchive::IsSaving() const noexcept
	{
		return m_IsSaving;
	}

	bool IArchive::IsTextFormat() const noexcept
	{
		return m_Format == EArchiveFormat::Text;
	}

	bool IArchive::IsValid() const noexcept
	{
		return m_IsOpen && m_IsValid;
	}

	uint64 IArchive::ProcessedBytes() const noexcept
	{
		return m_ProcessedBytes;
	}

	SaveArchive::SaveArchive(const Path& aPath, EArchiveFormat aFormat) noexcept
		: IArchive{aPath, aFormat }
	{
		BuildTempPath();

		std::ios_base::openmode mode = std::ios::out | std::ios::trunc;
		if (aFormat == EArchiveFormat::Binary)
			mode |= std::ios::binary;

		m_File.open(m_TempPath, mode);

		m_IsOpen = m_File.is_open();
		m_IsValid = m_IsOpen && m_File.good();
		m_IsSaving = true;
	}

	SaveArchive::~SaveArchive() noexcept
	{
		if (IsOpen())
		{
			m_File.flush();
			m_File.close();
		}

		if (IsValid() && ProcessedBytes() > 0u)
		{
			if (File::Exists(m_Path))
			{
				File::ClearAttributes(m_Path);
				File::Replace(m_TempPath, m_Path);
			}
			else
				File::Move(m_TempPath, m_Path, EFileMoveMode::NoOverWrite);

			if (m_SetFileHiddenOnDone)
				FilepathUtils::SetFileHidden(m_Path);
		}
		
		if (File::Exists(m_TempPath))
			File::Delete(m_TempPath);
	}

	bool SaveArchive::Flush() noexcept
	{
		RLS_ASSERT(IsOpen() && IsValid(), "[SaveArchive::Flush]: File is invalid.");
		
		m_File.flush();
		m_IsValid = m_File.good();
		
		return m_IsValid;
	}

	bool SaveArchive::ProcessRaw(void* aDataPtr, size_t aSize) noexcept
	{
		RLS_ASSERT(IsOpen() && IsValid(), "[SaveArchive::ProcessRaw]: File is invalid.");

		if (!IsOpen() || !IsValid())
			return false;

		m_File.write(static_cast<const char*>(aDataPtr), aSize);
		m_IsValid = m_File.good();
		m_ProcessedBytes += aSize;
		
		return m_IsValid;
	}

	bool SaveArchive::Seek(StreamPos aStreamPos) noexcept
	{
		m_File.seekp(aStreamPos);
		return m_File.good();
	}

	StreamPos SaveArchive::Tell() noexcept
	{
		return m_File.tellp();
	}

	void SaveArchive::SetFileHiddenOnDone(bool aState) noexcept
	{
		m_SetFileHiddenOnDone = aState;
	}

	void SaveArchive::BuildTempPath() noexcept
	{
		m_TempPath = m_Path;
		m_TempPath += ".tmp.";
		m_TempPath += StringUtils::ToWideString(Process::GetCurrentID());
		m_TempPath += ".";
		m_TempPath += StringUtils::ToWideString(s_ActiveCounter.fetch_add(1, std::memory_order_seq_cst));
	}

	LoadArchive::LoadArchive(const Path& aPath, EArchiveFormat aFormat) noexcept
		: IArchive{aPath, aFormat }
	{
		std::ios_base::openmode mode = std::ios::in;
		if (aFormat == EArchiveFormat::Binary)
			mode |= std::ios::binary;

		m_File.open(aPath, mode);
		m_IsOpen = m_File.is_open();
		m_IsValid = m_IsOpen && m_File.good();
		m_IsSaving = false;
	}

	LoadArchive::~LoadArchive() noexcept
	{
		if (IsOpen())
			m_File.close();
	}

	bool LoadArchive::Seek(StreamPos aStreamPos) noexcept
	{
		m_File.seekg(aStreamPos);
		return m_File.good();
	}

	StreamPos LoadArchive::Tell() noexcept
	{
		return m_File.tellg();
	}

	bool LoadArchive::ProcessRaw(void* aDataPtr, size_t aSize) noexcept
	{
		RLS_ASSERT(IsOpen() && IsValid(), "[LoadArchive::ProcessRaw]: File is invalid.");

		if (!IsOpen() || !IsValid())
			return false;

		m_File.read(static_cast<char*>(aDataPtr), aSize);
		m_IsValid = m_File.good();
		m_ProcessedBytes += aSize;

		return m_IsValid;
	}

}
