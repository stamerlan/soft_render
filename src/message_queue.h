#ifndef MSG_H_
#define MSG_H_

struct Message {
	enum class type {
		QUIT,
	} type;
};

#endif /* MSG_H_ */
