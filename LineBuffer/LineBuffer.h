#ifndef _LINEBUFFER_
#define _LINEBUFFER_

#include <algorithm>
#include <memory>

#include <CPUSupportChecker.h>

//AVX
# include <immintrin.h>
#define AVX_ALIEN_RANGE 32

namespace myLib {
	//�Q�l:https://qiita.com/Gaccho/items/e936de237676120aa8a0
	template<typename T>
	class LineBuffer_XY {
	public:
		constexpr LineBuffer_XY<T>(const size_t _id, const std::shared_ptr<T[]>& _arr)
			: m_id_xy(_id), m_arr_ref(_arr) {
		}
		constexpr T operator+() const { return +m_arr_ref[m_id_xy]; }
		constexpr T operator-() const { return -m_arr_ref[m_id_xy]; }
		constexpr T operator+(const T& _a) const { return m_arr_ref[m_id_xy] + _a; }
		constexpr T operator-(const T& _a) const { return m_arr_ref[m_id_xy] - _a; }
		constexpr T operator*(const T& _a) const { return m_arr_ref[m_id_xy] * _a; }
		constexpr T operator/(const T& _a) const { return m_arr_ref[m_id_xy] / _a; }
		constexpr T operator%(const T& _a) const { return m_arr_ref[m_id_xy] % _a; }
		constexpr T operator&(const T& _a) const { return m_arr_ref[m_id_xy] & _a; }
		constexpr T operator|(const T& _a) const { return m_arr_ref[m_id_xy] | _a; }
		constexpr T operator^(const T& _a) const { return m_arr_ref[m_id_xy] ^ _a; }
		constexpr T operator=(const T& _a) const { return m_arr_ref[m_id_xy] = _a; }
		constexpr T operator+=(const T& _a) const { return m_arr_ref[m_id_xy] += _a; }
		constexpr T operator-=(const T& _a) const { return m_arr_ref[m_id_xy] -= _a; }
		constexpr T operator*=(const T& _a) const { return m_arr_ref[m_id_xy] *= _a; }
		constexpr T operator/=(const T& _a) const { return m_arr_ref[m_id_xy] /= _a; }
		constexpr T operator%=(const T& _a) const { return m_arr_ref[m_id_xy] %= _a; }
		constexpr T operator^=(const T& _a) const { return m_arr_ref[m_id_xy] ^= _a; }
		constexpr T operator|=(const T& _a) const { return m_arr_ref[m_id_xy] |= _a; }
		constexpr T operator&=(const T& _a) const { return m_arr_ref[m_id_xy] &= _a; }
		constexpr T operator++() const { return ++m_arr_ref[m_id_xy]; }
		constexpr T operator++(int) const { return m_arr_ref[m_id_xy]++; }
		constexpr T operator--() const { return --m_arr_ref[m_id_xy]; }
		constexpr T operator--(int) const { return m_arr_ref[m_id_xy]--; }
		constexpr T* operator&()const { return &m_arr_ref[m_id_xy]; }
		constexpr const bool operator!() const { return !m_arr_ref[m_id_xy]; }
		constexpr const bool operator&&(const T& _a) const { return (m_arr_ref[m_id_xy] && _a); }
		constexpr const bool operator||(const T& _a) const { return (m_arr_ref[m_id_xy] || _a); }
		constexpr const bool operator<(const T& _a) const { return (m_arr_ref[m_id_xy] < _a); }
		constexpr const bool operator>(const T& _a) const { return (m_arr_ref[m_id_xy] > _a); }
		constexpr const bool operator<=(const T& _a) const { return (m_arr_ref[m_id_xy] <= _a); }
		constexpr const bool operator>=(const T& _a) const { return (m_arr_ref[m_id_xy] >= _a); }
		constexpr const bool operator!=(const T& _a) const { return (m_arr_ref[m_id_xy] != _a); }
		constexpr const bool operator==(const T& _a) const { return (m_arr_ref[m_id_xy] == _a); }

		operator T() const { return m_arr_ref[m_id_xy]; }

	private:
		//������2�����z��ŏI�I�Ȕz��ԍ��Ɍ����Ă�1�����z��̍��W������
		const size_t m_id_xy = 0;
		//1�����z��̎Q��(LineBuffer.t�̎Q��)
		const std::shared_ptr<T[]>& m_arr_ref;
	};

	template<typename T>
	class LineBuffer_X {
	public:
		constexpr LineBuffer_X<T>(const size_t _id, const std::shared_ptr<T[]>& _arr) : m_id_x(_id), m_arr_ref(_arr) {}
		//(2�ڂ�[])
		constexpr const LineBuffer_XY<T> operator[](const size_t _x) const {
			const LineBuffer_XY<T> arr_xy(m_id_x + _x, m_arr_ref);
			return arr_xy;
		}
		constexpr const LineBuffer_XY<T> at(const size_t _x) const {
			const LineBuffer_XY<T> arr_xy(m_id_x + _x, m_arr_ref);
			return arr_xy;
		}
		constexpr const LineBuffer_XY<T> front() const {
			const LineBuffer_XY<T> arr_xy(m_id_x, m_arr_ref);
			return arr_xy;
		}

	private:
		//�����ɂ�2�����z���x���擪�Ɍ����Ă�1�����z��̍��W������
		const size_t m_id_x = 0;
		//1�����z��̎Q��(LineBuffer.t�̎Q��)
		const std::shared_ptr<T[]>& m_arr_ref;
	};

	template<typename T>
	class LineBuffer {
	public:
		LineBuffer<T>() {}
		//x*y��1�����z��𐶐����Ă��ׂĂ̔z���value�ɂ���
		LineBuffer<T>(const size_t _y, const size_t _x, const T _value) {
			create(_y, _x);
			zeroclear();
			std::memset(m_arr_real.get(), _value, sizeof(T) * capacity());
		}

		constexpr LineBuffer<T>(const LineBuffer<T>& _src) {
			if (capacity() != _src.capacity()) {
				Reset();
				create(_src.sizeY(), _src.sizeX());
				zeroclear();
			}
			memcpy_s(m_arr_real.get(), sizeof(T) * capacity(), _src.m_arr_real.get(), sizeof(T) * _src.capacity());
		}

		//x*y��1�����z��𐶐����邾��
		constexpr LineBuffer<T>(const size_t _y, const size_t _x) {
			create(_y, _x);
			zeroclear();
		}

		//2�����z��Ɍ����Ă�1�����z���x���̐擪��LineBuffer_X�����(1�ڂ�[])
		constexpr const LineBuffer_X<T> operator[](const size_t _y) const {
			LineBuffer_X<T> arr_x(_y * capacityX(), m_arr_real);
			return arr_x;
		}
		constexpr const LineBuffer<T>& operator =(const LineBuffer<T>& _src) {
			if (capacity() != _src.capacity()) {
				Reset();
				create(_src.sizeY(), _src.sizeX());
				zeroclear();
			}
			memcpy_s(m_arr_real.get(), sizeof(T) * capacity(), _src.m_arr_real.get(), sizeof(T) * _src.capacity());
			return *this;
		}

		void Reset() {
			m_arr_real.reset();
			m_Capacity_x = m_Size_x = m_Size_y = 0;
		}

		constexpr T* operator&() const {
			return m_arr_real.get();
		}

		constexpr const LineBuffer_X<T> at(const size_t _y) const {
			return LineBuffer_X(_y * capacityX(), m_arr_real);
		}
		constexpr const LineBuffer_X<T> front() const {
			return LineBuffer_X(0, m_arr_real);
		}
		constexpr size_t size() const {
			return m_Size_x * m_Size_y;
		}
		constexpr size_t capacity() const {
			return m_Capacity_x * m_Size_y;
		}
		constexpr size_t sizeX() const {
			return m_Size_x;
		}
		constexpr size_t capacityX() const {
			return m_Capacity_x;
		}
		constexpr size_t sizeY() const {
			return m_Size_y;
		}
		constexpr T* const get() const {
			return m_arr_real.get();
		}

		void resize(const size_t _y, const size_t _x) {
			m_arr_real.reset();
			create(_y, _x);
			zeroclear();
		}

		void zeroclear() {
			std::memset(m_arr_real.get(), NULL, sizeof(T) * capacity());
		}

		bool empty() {
			return m_arr_real ? false : true;
		}

		void swap(LineBuffer<T>& _swap) {
			size_t tempx = m_Size_x, tempy = m_Size_y, tempRx = m_Capacity_x;
			m_Size_x = _swap.m_Size_x;
			m_Size_y = _swap.m_Size_y;
			m_Capacity_x = _swap.m_Capacity_x;

			_swap.m_Size_x = tempx;
			_swap.m_Size_y = tempy;
			_swap.m_Capacity_x = tempRx;

			std::swap(m_arr_real, _swap.m_arr_real);
		}

		void add(LineBuffer<float>& _add) {
			if (CPUSupportChecker::AVX2()) {
				avx_add(_add);
			}
			else {
				sequential_add(_add);
			}
		}

		void mul(float _mul) {
			if (CPUSupportChecker::AVX2()) {
				avx_mul(_mul);
			}
			else {
				sequential_mul(_mul);
			}
		}

		void abs() {
			sequential_abs();
		}

		T Max() {
			return *std::max_element(m_arr_real.get(), m_arr_real.get() + capacity());
		}

		T Min() {
			return *std::min_element(m_arr_real.get(), m_arr_real.get() + capacity());
		}

	private:
		std::shared_ptr<T[]> m_arr_real;
		size_t m_Size_x = 0, m_Size_y = 0, m_Capacity_x = 0;

		void create(const size_t _y, const size_t _x) {
			m_Size_x = _x == 0 ? 1 : _x;
			m_Size_y = _y == 0 ? 1 : _y;
			m_Capacity_x = calcAlienRange(m_Size_x);
			m_arr_real = std::make_unique<T[]>(m_Capacity_x * m_Size_y);
		}

		void avx_add(LineBuffer<float>& _add) {
			const size_t processFrames = std::min BOOST_PREVENT_MACRO_SUBSTITUTION(this->capacityX(), _add.capacityX());
			const size_t avxForAdd = sizeof(__m256) / sizeof(float);

			for (int32_t chan = 0; chan < sizeY(); ++chan) {
				float* ptrThis = &at(chan).front();
				float* ptrAdd = &_add.at(chan).front();
				for (int32_t fram = 0; fram < processFrames; fram += avxForAdd) {
					__m256 avxThis = _mm256_load_ps(ptrThis);
					__m256 avxAdd = _mm256_load_ps(ptrAdd);
					_mm256_store_ps(ptrThis, _mm256_add_ps(avxThis, avxAdd));
					ptrThis += avxForAdd;
					ptrAdd += avxForAdd;
				}
			}
		}

		void sequential_add(LineBuffer<float>& _add) {
			const size_t processFrames = std::min BOOST_PREVENT_MACRO_SUBSTITUTION(this->sizeX(), _add.sizeX());

			for (int32_t chan = 0; chan < sizeY(); ++chan) {
				float* ptrThis = &at(chan).front();
				float* ptrAdd = &_add.at(chan).front();
				for (int32_t fram = 0; fram < processFrames; ++fram) {
					*ptrThis += *ptrAdd;
					ptrThis++;
					ptrAdd++;
				}
			}
		}

		void avx_mul(float _mul) {
			const int32_t avxForAdd = sizeof(__m256) / sizeof(float);
			const __m256 mul = _mm256_set_ps(_mul, _mul, _mul, _mul, _mul, _mul, _mul, _mul);

			for (int32_t chan = 0; chan < sizeY(); ++chan) {
				float* ptrThis = &at(chan).front();
				for (int32_t fram = 0; fram < capacityX(); fram += avxForAdd) {
					__m256 avxThis = _mm256_load_ps(ptrThis);
					_mm256_store_ps(ptrThis, _mm256_mul_ps(avxThis, mul));
					ptrThis += avxForAdd;
				}
			}
		}

		void sequential_mul(float _mul) {
			for (int32_t chan = 0; chan < sizeY(); ++chan) {
				float* ptrThis = &at(chan).front();
				for (int32_t fram = 0; fram < sizeX(); ++fram) {
					*ptrThis *= _mul;
					ptrThis++;
				}
			}
		}

		void sequential_abs() {
			float* ptrThis = get();
			for (int32_t i = 0; i < capacity(); ++i) {
				*ptrThis = std::fabsf(*ptrThis++);
			}
		}

		size_t calcAlienRange(size_t _range) {
			auto mod = _range % AVX_ALIEN_RANGE;
			return _range + (AVX_ALIEN_RANGE - mod);
		}
	};
};
//�v�f�ԍ��T����((y*m_Size_x)+x)

#endif