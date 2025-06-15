#ifndef _MSTDPTRHELPER_
#define _MSTDPTRHELPER_

#include <memory>

/*�v���C�x�[�g�ȃR���X�g���N�^�A�f�X�g���N�^�̃N���X��shared_ptr��make_shared�͂ł��Ȃ�
(friend���ꂽ�N���X�����friendClass->shared_ptr->�����N���X�̏��ŃA�N�Z�X���������邽�ߕs��)
����
�R���X�g���N�^�������Ă���Ƃ���ɕ⏕�I�u�W�F�N�g���`�������makeshared������⏕�I�u�W�F�N�g����
�g�p�I�u�W�F�N�g�ւ�shared_ptr���쐬
���ړI�N���X(T)���p�������⏕�I�u�W�F�N�g(entry)��ړI�N���X�Ƀ|�����[�t�����n���B
�f�����b�g
�A���P�[�g��������StdPtrHelper::entry+<T>�̗ʂɂȂ�
*/
namespace myLib {
	template<typename T>
	class MemPtrHelper {
		friend T;

		struct entity : public T {
			template<typename... Args>
			entity(Args&&... args) : T(std::forward<Args>(args)...) {}
		};

		template<typename... Args>
		static std::shared_ptr<T> make_shared(Args&&... args) {
			return std::make_shared<entity>(std::forward<Args>(args)...);
		}

		template<typename Alloc, typename... Args>
		static std::shared_ptr<T> allocate_shared(const Alloc& alloc, Args&&... args) {
			return std::allocate_shared<entity>(alloc, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static std::unique_ptr<T> make_unique(Args&&... args) {
			return std::make_unique<entity>(std::forward<Args>(args)...);
		}
	};
};
#endif