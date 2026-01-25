#pragma once

namespace Relentless
{
	template<typename T, typename IndexT = uint32_t>
	class DenseSet
	{
		static_assert(std::is_integral_v<T>, "DenseSet expects an integral ID type.");
		static_assert(std::is_unsigned_v<T>, "DenseSet expects an unsigned ID type.");

	public:
		static constexpr IndexT INVALID = std::numeric_limits<IndexT>::max();

		NO_DISCARD bool Contains(T v) const noexcept
		{
			const IndexT sv = (v < m_Sparse.size()) ? m_Sparse[(IndexT)v] : INVALID;
			return sv != INVALID && sv < (IndexT)m_Dense.size() && m_Dense[sv] == v;
		}

		bool Insert(T v) noexcept
		{
			EnsureSparse(v);
			if (Contains(v))
				return false;

			m_Sparse[(IndexT)v] = (IndexT)m_Dense.size();
			m_Dense.push_back(v);
			return true;
		}

		bool Erase(T v) noexcept
		{
			if (!Contains(v))
				return false;

			const IndexT idx = m_Sparse[(IndexT)v];
			const T last = m_Dense.back();

			m_Dense[idx] = last;
			m_Sparse[(IndexT)last] = idx;

			m_Dense.pop_back();
			m_Sparse[(IndexT)v] = INVALID;
			return true;
		}

		bool Replace(T from, T to) noexcept
		{
			if (from == to)
				return true;

			if (!Contains(from))
				return false;

			EnsureSparse(to);
			if (Contains(to))
				return false;

			const IndexT idx = m_Sparse[(IndexT)from];

			m_Dense[idx] = to;
			m_Sparse[(IndexT)to] = idx;
			m_Sparse[(IndexT)from] = INVALID;
			return true;
		}

		void Clear() noexcept
		{
			m_Dense.clear();
			m_Sparse.clear();
		}

		const std::vector<T>& Dense() const noexcept { return m_Dense; }
		std::vector<T>& Dense() noexcept { return m_Dense; }

		NO_DISCARD IndexT Size() const noexcept { return (IndexT)m_Dense.size(); }
		NO_DISCARD bool Empty() const noexcept { return m_Dense.empty(); }

	private:
		void EnsureSparse(T v) noexcept
		{
			const IndexT iv = (IndexT)v;
			if (iv >= (IndexT)m_Sparse.size())
				m_Sparse.resize((size_t)iv + 1u, INVALID);
		}

	private:
		std::vector<T> m_Dense;
		std::vector<IndexT> m_Sparse;
	};
}