#ifndef MSG_H_
#define MSG_H_

struct Message {
	enum class type {
		QUIT,
		MOVE_FURTHER,
		MOVE_CLOSER,
		MOVE_LEFT,
		MOVE_RIGHT,
	} type;
};

#endif /* MSG_H_ */
