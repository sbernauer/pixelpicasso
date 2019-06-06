#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/sendfile.h>
#include <netdb.h>
#include <assert.h>

#include "image.h"
#include "network.h"


#define NUM_TEXT_DEFAULT 65536
#define NUM_TEXT_BLOCK 65536

static int one = 1;

void net_frame_free(struct net_frame* frame) {
	free(frame->data);
	free(frame->cmds);
}

int net_frame_to_net_frame(struct net_frame* ret, struct img_frame* src, unsigned int width, unsigned int height, bool monochrome, unsigned int offset_x, unsigned int offset_y, unsigned int sparse_perc, unsigned int penId) {
	int err = 0;
	size_t data_alloc_size, max_print_size, i;
	ssize_t print_size;
	struct pf_cmd* commands, *cmd;
	char* data, *data_tmp;
	off_t offset = 0;
	unsigned int x = 0, y = 0;
	struct img_pixel pixel;
	struct net_frame* dst = malloc(sizeof(struct net_frame));
	if(!dst) {
		err = -ENOMEM;
		goto fail;
	}

	int commandCounter = 0;
	while(y < width) {
			if (y % 2 == 0) {
				while(x < height - 1) {
					commandCounter++;
					x++;
				}
			} else {
				while(x > 0) {
					commandCounter++;
					x--;
				}
			}
			commandCounter++;
			y++;
	}
	y--; // Because of increment at end of loop
	while(y > 0) {
		commandCounter++;
		y--;
	}

	// Move a bit up to compensate
	commandCounter += 2;

	x = 0;
	y = 0; // Reset after calculation


	dst->width = width;
	dst->height = height;
	dst->duration_ms = src->duration_ms;
	dst->num_cmds = commandCounter;

	commands = malloc(dst->num_cmds * sizeof(struct pf_cmd));
	if(!commands) {
		err = -ENOMEM;
		goto fail_frame_alloc;
	}
	dst->cmds = commands;

	data_alloc_size = NUM_TEXT_DEFAULT;
	data = malloc(data_alloc_size);
	if(!data) {
		err = -ENOMEM;
		goto fail_commands_alloc;
	}

	long bufferCounter = 0;
	while(y < width) { // Dont print last line to save space in the buffer for returning to (0,0)
		if (y % 2 == 0) {
			while(x < height - 1) {


				while(true) {
					max_print_size = data_alloc_size - offset;
					pixel = src->pixels[y * width + x];
					print_size = snprintf(data + offset, data_alloc_size - offset, "MOVE %d 0 1 %06x 0\n", penId, pixel.abgr >> 8);
					if(print_size < 0) {
						err = -EINVAL;
						goto fail_data_alloc;
					}
					if(print_size < max_print_size) {
						// First part of command setup
						// We can't setup .data or .cmd here because data might be realloced
						cmd = &commands[bufferCounter];
						cmd->offset = offset;
						cmd->length = print_size;
						offset += print_size;
						bufferCounter++;
						break;
					}
					data_alloc_size += NUM_TEXT_BLOCK;
					data_tmp = realloc(data, data_alloc_size);
					if(!data_tmp) {
						err = -ENOMEM;
						goto fail_data_alloc;
					}
					data = data_tmp;
				}



				x++;
			}
		} else {
			while(x > 0) {


				while(true) {
					max_print_size = data_alloc_size - offset;
					pixel = src->pixels[y * width + x];
					print_size = snprintf(data + offset, data_alloc_size - offset, "MOVE %d 0 -1 %06x 0\n", penId, pixel.abgr >> 8);
					if(print_size < 0) {
						err = -EINVAL;
						goto fail_data_alloc;
					}
					if(print_size < max_print_size) {
						// First part of command setup
						// We can't setup .data or .cmd here because data might be realloced
						cmd = &commands[bufferCounter];
						cmd->offset = offset;
						cmd->length = print_size;
						offset += print_size;
						bufferCounter++;
						break;
					}
					data_alloc_size += NUM_TEXT_BLOCK;
					data_tmp = realloc(data, data_alloc_size);
					if(!data_tmp) {
						err = -ENOMEM;
						goto fail_data_alloc;
					}
					data = data_tmp;
				}



				x--;
			}
		}



				while(true) {
					max_print_size = data_alloc_size - offset;
					pixel = src->pixels[y * width + x];
					print_size = snprintf(data + offset, data_alloc_size - offset, "MOVE %d 1 0 %06x 0\n", penId, pixel.abgr >> 8);
					if(print_size < 0) {
						err = -EINVAL;
						goto fail_data_alloc;
					}
					if(print_size < max_print_size) {
						// First part of command setup
						// We can't setup .data or .cmd here because data might be realloced
						cmd = &commands[bufferCounter];
						cmd->offset = offset;
						cmd->length = print_size;
						offset += print_size;
						bufferCounter++;
						break;
					}
					data_alloc_size += NUM_TEXT_BLOCK;
					data_tmp = realloc(data, data_alloc_size);
					if(!data_tmp) {
						err = -ENOMEM;
						goto fail_data_alloc;
					}
					data = data_tmp;
				}


		y++;
	}
	y--; // Because of increment at end of loop

	assert(x == 0);
	assert(y == height - 1);

	while(y > 0) {
		y--;


		while(true) {
					max_print_size = data_alloc_size - offset;
					pixel = src->pixels[y * width + x];
					print_size = snprintf(data + offset, data_alloc_size - offset, "MOVE %d -1 0 %06x 0\n", penId, pixel.abgr >> 8);
					if(print_size < 0) {
						err = -EINVAL;
						goto fail_data_alloc;
					}
					if(print_size < max_print_size) {
						// First part of command setup
						// We can't setup .data or .cmd here because data might be realloced
						cmd = &commands[bufferCounter];
						cmd->offset = offset;
						cmd->length = print_size;
						offset += print_size;
						bufferCounter++;
						break;
					}
					data_alloc_size += NUM_TEXT_BLOCK;
					data_tmp = realloc(data, data_alloc_size);
					if(!data_tmp) {
						err = -ENOMEM;
						goto fail_data_alloc;
					}
					data = data_tmp;
				}



	}

	// Move a bit up to compensate
	for (int i = 0; i < 2; i++) {



		while(true) {
					max_print_size = data_alloc_size - offset;
					print_size = snprintf(data + offset, data_alloc_size - offset, "MOVE %d -1 0 000000 0\n", penId);
					if(print_size < 0) {
						err = -EINVAL;
						goto fail_data_alloc;
					}
					if(print_size < max_print_size) {
						// First part of command setup
						// We can't setup .data or .cmd here because data might be realloced
						cmd = &commands[bufferCounter];
						cmd->offset = offset;
						cmd->length = print_size;
						offset += print_size;
						bufferCounter++;
						break;
					}
					data_alloc_size += NUM_TEXT_BLOCK;
					data_tmp = realloc(data, data_alloc_size);
					if(!data_tmp) {
						err = -ENOMEM;
						goto fail_data_alloc;
					}
					data = data_tmp;
				}

	}

	fprintf(stderr, "\nAsserting %ld == %ld \n", bufferCounter, (long)dst->num_cmds);
	assert(bufferCounter == (long)dst->num_cmds);

	dst->data = data;

	for(i = 0; i < dst->num_cmds; i++) {
		// Second part of command setup
		// data is now finalized, we can set .data and calculate .cmd
		cmd = &commands[i];
		cmd->data = data;
		cmd->cmd = data + cmd->offset;
	}

	*ret = *dst;

	return 0;

fail_data_alloc:
	free(data);
fail_commands_alloc:
	free(commands);
fail_frame_alloc:
	free(dst);
fail:
	return err;
}

void net_free_animation(struct net_animation* anim) {
	size_t i;
	for(i = 0; i < anim->num_frames; i++) {
		net_frame_free(&anim->frames[i]);
	}
	free(anim->frames);
	free(anim);
}

int net_animation_to_net_animation(struct net_animation** ret, struct img_animation* src, bool monochrome, unsigned int offset_x, unsigned int offset_y, unsigned int sparse_perc, progress_cb progress_cb, unsigned int penId) {
	int err = 0;
	size_t i;
	struct timespec last_progress;
	struct net_animation* dst = malloc(sizeof(struct net_animation));
	if(!dst) {
		err = -ENOMEM;
		goto fail;
	}
	dst->num_frames = 0;

	dst->frames = malloc(src->num_frames * sizeof(struct net_frame));
	if(!dst->frames) {
		err = -ENOMEM;
		goto fail_animation_alloc;
	}

	if(progress_cb) {
		last_progress = progress_limit_rate(progress_cb, 0, src->num_frames, PROGESS_INTERVAL_DEFAULT, NULL);
	}
	for(i = 0; i < src->num_frames; i++) {
		err = net_frame_to_net_frame(&dst->frames[i], &src->frames[i], src->width, src->height, monochrome, offset_x, offset_y, sparse_perc, penId);
		dst->num_frames++;
		if(err) {
			goto fail_frames_alloc;
		}
		if(progress_cb) {
			last_progress = progress_limit_rate(progress_cb, dst->num_frames, src->num_frames, PROGESS_INTERVAL_DEFAULT, &last_progress);
		}
	}

	*ret = dst;
	return 0;

fail_frames_alloc:
	for(i = 0; i < dst->num_frames; i++) {
		net_frame_free(&dst->frames[i]);
	}
//fail_animation_frames_alloc:
	free(dst->frames);
fail_animation_alloc:
	free(dst);
fail:
	return err;
}

int net_alloc(struct net** ret) {
	int err = 0;
	struct net* net = malloc(sizeof(struct net));
	if(!net) {
		err = -ENOMEM;
		goto fail;
	}
	net->num_src_addresses = 0;
	net->state = NET_STATE_IDLE;
	net->ignore_broken_pipe = false;

	*ret = net;

fail:
	return err;
}

void net_free(struct net* net) {
	assert(net->state == NET_STATE_IDLE || net->state == NET_STATE_SHUTDOWN);

	free(net->threads_send);
	free(net->targs_send);
	free(net);
}

static void* net_send_thread(void* data) {
	int err = 0, sock;
	struct net_threadargs_send* args = data;
	off_t initial_offset, offset;
	size_t length, cmds_per_thread, num_cmds;
	struct net_frame* frame;
	ssize_t write_size;
	unsigned int thread_id = args->thread_id;
	struct net* net = args->net;

reconnect:
	sock = socket(args->remoteaddr->ss_family, SOCK_STREAM, 0);
	if(sock < 0) {
		err = sock;
		goto fail;
	}

	if((err = connect(sock, (struct sockaddr*)args->remoteaddr, args->remoteaddr_len))) {
		fprintf(stderr ,"Failed to connect: %s\n", strerror(-err));
		goto fail;
	}

	while(true) {
		frame = net->current_frame;
		num_cmds = frame->num_cmds;
		cmds_per_thread = num_cmds / net->num_send_threads;
		initial_offset = frame->cmds[thread_id * cmds_per_thread].offset;
		length = frame->cmds[(thread_id + 1) * cmds_per_thread - 1].offset - initial_offset;
		offset = 0;
		// fprintf(stderr, "Flood %ld commands from %ld with length %ld\n", cmds_per_thread, initial_offset, length);
		assert((long)length > offset);
		while(offset < length) {
			write_size = write(sock, frame->data + initial_offset + offset, length - offset);
			if(write_size < 0) {
				err = -errno;
				if(errno == EPIPE && net->ignore_broken_pipe) {
					continue;
				}

				if(errno == ECONNRESET)
					goto newsocket;

				goto fail;
			}
			offset += write_size;
		}

		// // Read incoming data to prevent bufffer from getting full
		// int* buffer = 0;
		// int len = 0;
		// ioctl(sock, FIONREAD, &len);
		// if (len > 0) {
		// 	// This will consume up to buf_size bytes from the socket descriptor sock_fd without actually copying them to the buffer. 
		// 	int n = recv(sock, buffer, len, MSG_TRUNC);
		// 	fprintf(stderr, "Read %d bytes\n", n);
		// }
		// break; // TODO Delete break to make infinite loop
	}

fail:
	shutdown(sock, SHUT_RDWR);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
	close(sock);
	doshutdown(err);
	return NULL;
newsocket:
	shutdown(sock, SHUT_RDWR);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
	close(sock);
	goto reconnect;
}

static void* net_animate_thread(void* data) {
	struct net_threadargs_animate* args = data;
	size_t num_frames = args->anim->num_frames, frame_id = 0;
	while(true) {
		// Since POSIX.1-2008 usleep is no cancellation point anymore
		pthread_testcancel();
		args->net->current_frame = &args->anim->frames[frame_id];
		usleep(args->net->current_frame->duration_ms * 1000UL);
		frame_id++;
		frame_id %= num_frames;
	}
	return NULL;
}

int net_send_animation(struct net* net, struct sockaddr_storage* dst_address, size_t dstaddr_len, unsigned int num_threads, struct net_animation* anim) {
	int err = 0;
	unsigned int i;

	assert(net->state == NET_STATE_IDLE);
	net->state = NET_STATE_SENDING;

	net->current_frame = &anim->frames[0];

	net->threads_send = malloc(num_threads * sizeof(pthread_t));
	if(!net->threads_send) {
		err = -ENOMEM;
		goto fail;
	}

	net->targs_send = malloc(num_threads * sizeof(struct net_threadargs_send));
	if(!net->targs_send) {
		err = -ENOMEM;
		goto fail_threads_alloc;
	}

	net->num_send_threads = 0;
	for(i = 0; i < num_threads; i++) {
		net->targs_send[i].net = net;
		// TODO Duplicate address
		net->targs_send[i].remoteaddr = dst_address;
		net->targs_send[i].remoteaddr_len = dstaddr_len;
		net->targs_send[i].thread_id = i;

		err = -pthread_create(&net->threads_send[i], NULL, net_send_thread, &net->targs_send[i]);
		if(err) {
			goto fail_thread_create;
		}
		net->num_send_threads++;
	}

	net->targs_animate.net = net;
	net->targs_animate.anim = anim;

	err = -pthread_create(&net->thread_animate, NULL, net_animate_thread, &net->targs_animate);
	if(err) {
		goto fail_thread_create;
	}

	return 0;

fail_thread_create:
	for(i = 0; i < net->num_send_threads; i++) {
		pthread_cancel(net->threads_send[i]);
		pthread_join(net->threads_send[i], NULL);
	}
//fail_targs_alloc:
	free(net->targs_send);
fail_threads_alloc:
	free(net->threads_send);
fail:
	net->state = NET_STATE_IDLE;
	return err;
}

void net_shutdown(struct net* net) {
	unsigned int i;

	assert(net->state == NET_STATE_SENDING);

	pthread_cancel(net->thread_animate);
	pthread_join(net->thread_animate, NULL);

	for(i = 0; i < net->num_send_threads; i++) {
		pthread_cancel(net->threads_send[i]);
		pthread_join(net->threads_send[i], NULL);
	}

	net->state = NET_STATE_SHUTDOWN;
}
