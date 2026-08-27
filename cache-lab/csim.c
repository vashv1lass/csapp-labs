// Implemented by Maxim vashv1las Vashkevich
// Started on August 27th, 2026, at 11:28AM (UTC+3)

#include "cachelab.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Cache logic implementation
typedef struct cache_line {
	uint64_t tag;
	bool valid_bit;
	uint64_t accessed_ago;
} cache_line_t;

cache_line_t create_cache_line() {
	cache_line_t cache_line;
	
	cache_line.tag = 0;
	cache_line.valid_bit = false;
	cache_line.accessed_ago = 0;

	return cache_line;
}

void destroy_cache_line(cache_line_t * line) {
	line->tag = 0;
	line->valid_bit = false;
	line->accessed_ago = 0;
}

typedef struct cache_set {
	cache_line_t * lines;
} cache_set_t;

cache_set_t create_cache_set(size_t setsz) {
	cache_set_t cache_set;

	cache_set.lines = (cache_line_t *)malloc(setsz * sizeof(cache_line_t));
	for (size_t i = 0; i < setsz; i++) { // setsz was my first f up - corrected with LLM hint
		cache_set.lines[i] = create_cache_line();
	}

	return cache_set;
}

void destroy_cache_set(cache_set_t * set, size_t setsz) {
	for (size_t i = 0; i < setsz; i++) {
		destroy_cache_line(&set->lines[i]);
	}
	free(set->lines); // free(set) was my second f up - corrected with LLM hint
	set = NULL;
}

typedef struct cache {
	size_t setcnt;
	size_t assoc;
	size_t linesize;

	cache_set_t * sets;
} cache_t;

cache_t create_cache(size_t set_count, size_t associativity, size_t line_size) {
	cache_t cache;
	
	cache.setcnt = set_count;
	cache.assoc = associativity;
	cache.linesize = line_size;
	cache.sets = (cache_set_t *)malloc(set_count * sizeof(cache_set_t));
	for (size_t i = 0; i < set_count; i++) {
		cache.sets[i] = create_cache_set(associativity);
	}

	return cache;
}

void destroy_cache(cache_t * cache) {
	for (size_t i = 0; i < cache->setcnt; i++) {
		destroy_cache_set(&cache->sets[i], cache->assoc);
	}
	free(cache->sets);
	cache->sets = NULL;
	cache->setcnt = 0;
	cache->assoc = 0;
	cache->linesize = 0;
}

typedef struct address {
	uint64_t offset;
	uint64_t set_index;
	uint64_t tag;
} address_t;

uint64_t uint64_log2(uint64_t x) {
	return 63LL - __builtin_clzll(x);
}

address_t decompose_address(cache_t cache, uint64_t addr) {
	int8_t offset_bits = uint64_log2(cache.linesize);
	int8_t set_bits = uint64_log2(cache.setcnt);

	uint64_t offset_mask = (uint64_t)-1 >> (64 - offset_bits);
	uint64_t set_mask = (uint64_t)-1 >> (64 - set_bits) << offset_bits;
	uint64_t tag_mask = ~(offset_mask | set_mask);

	address_t decomposed_addr;
	decomposed_addr.offset = addr & offset_mask;
	decomposed_addr.set_index = (addr & set_mask) >> offset_bits;
	decomposed_addr.tag = (addr & tag_mask) >> (set_bits + offset_bits);

	return decomposed_addr;
}

typedef struct report {
	int32_t hit;
	int32_t miss;
	int32_t eviction;
} report_t;

report_t create_report() {
	report_t report;

	report.hit = 0;
	report.miss = 0;
	report.eviction = 0;

	return report;
}

void simulate_cache_access(cache_t * cache, uint64_t addr, int32_t size, report_t * report) {
	address_t addr_decomposed = decompose_address(*cache, addr);

	bool hit = false;
	bool no_evict = false;
	
	cache_set_t set = cache->sets[addr_decomposed.set_index];
	for (size_t i = 0; i < cache->assoc; i++) {
		if (set.lines[i].tag == addr_decomposed.tag && set.lines[i].valid_bit) {
			hit = true;
			set.lines[i].accessed_ago = (uint64_t)-1;
		}
		no_evict |= !set.lines[i].valid_bit;
		
		set.lines[i].accessed_ago++;
	}

	if (!hit) {
		if (!no_evict) {
			uint64_t max_accessed_ago = 0;
			cache_line_t * line_to_evict = NULL;
			for (size_t i = 0; i < cache->assoc; i++) {
				if (max_accessed_ago < set.lines[i].accessed_ago) {
					max_accessed_ago = set.lines[i].accessed_ago;
					line_to_evict = &set.lines[i];
				}
			}

			line_to_evict->accessed_ago = 0;
			line_to_evict->tag = addr_decomposed.tag;
			line_to_evict->valid_bit = true;
		} else {
			for (size_t i = 0; i < cache->assoc; i++) {
				if (!set.lines[i].valid_bit) {
					set.lines[i].accessed_ago = 0;
					set.lines[i].valid_bit = true;
					set.lines[i].tag = addr_decomposed.tag;
					
					break;
				}
			}
		}
	}

	report->hit += hit;
	report->miss += !hit;
	report->eviction += !(no_evict || hit);
}

void simulate_cache_load(cache_t * cache, uint64_t addr, int32_t size, report_t * report, bool verbose) {
	report_t old_state = *report;
	simulate_cache_access(cache, addr, size, report);
	bool hit = report->hit > old_state.hit;
	bool eviction = report->eviction > old_state.eviction;
	
	if (verbose) {
		printf("L %lx,%d %s%s\n", addr, size, hit ? "hit" : "miss", eviction ? " eviction" : "");
	}
}

void simulate_cache_store(cache_t * cache, uint64_t addr, int32_t size, report_t * report, bool verbose) {
	report_t old_state = *report;
	simulate_cache_access(cache, addr, size, report);
	bool hit = report->hit > old_state.hit;
	bool eviction = report->eviction > old_state.eviction;
	
	if (verbose) {
		printf("S %lx,%d %s%s\n", addr, size, hit ? "hit" : "miss", eviction ? " eviction" : "");
	}
}

void simulate_cache_modify(cache_t * cache, uint64_t addr, int32_t size, report_t * report, bool verbose) {
	report_t old_state = *report;

	simulate_cache_access(cache, addr, size, report);
	bool load_hit = report->hit > old_state.hit;
	bool load_evicted = report->eviction > old_state.eviction;

	simulate_cache_access(cache, addr, size, report);

	if (verbose) {
		printf("M %lx,%d %s%s hit\n", addr, size, load_hit ? "hit" : "miss", load_evicted ? " eviction" : "");
	}
}

report_t simulate_cache(cache_t * cache, char tracefile[64], bool verbose) {
	report_t report = create_report();

	FILE * tracefile_d = fopen(tracefile, "r");
	char trace_access_buffer[64];
	while (fgets(trace_access_buffer, sizeof(trace_access_buffer), tracefile_d) != NULL) {
		size_t trace_access_shift = trace_access_buffer[0] == ' ';

		char op = trace_access_buffer[trace_access_shift];
		char * base_addr_end;
		uint64_t base_addr = (uint64_t)strtoull(trace_access_buffer + trace_access_shift + 1, &base_addr_end, 16);
		int32_t bytes_accessed = (int32_t)strtol(base_addr_end + 1, NULL, 10);

		switch (op) {
		case 'L':
			simulate_cache_load(cache, base_addr, bytes_accessed, &report, verbose);
			break;
		case 'S':
			simulate_cache_store(cache, base_addr, bytes_accessed, &report, verbose);
			break;
		case 'M':
			simulate_cache_modify(cache, base_addr, bytes_accessed, &report, verbose);
			break;
		default:
			break;
		}
	}

	return report;
}

void print_help_message() {
	printf(
	    "Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
	    "Options:\n"
	    "  -h         Print this help message.\n"
	    "  -v         Optional verbose flag.\n"
	    "  -s <num>   Number of set index bits.\n"
	    "  -E <num>   Number of lines per set.\n"
	    "  -b <num>   Number of block offset bits.\n"
	    "  -t <file>  Trace file.\n\n"
	    "Examples:\n"
	    "  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.tracen\n"
	    "  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n"
	);
}

void parse_clargs(int argc, char * const * argv, bool * verbose_flag, int * set_index_bits, int * associativity, int * offset_bits, char valgrind_trace_path[64]) {
	int opt = 0;
	while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (opt) {
		case 'h':
			print_help_message();
			exit(EXIT_SUCCESS);
		case 'v':
			*verbose_flag = true;
			break;
		case 's':
			*set_index_bits = atoi(optarg);
			break;
		case 'E':
			*associativity = atoi(optarg);
			break;
		case 'b':
			*offset_bits = atoi(optarg);
			break;
		case 't':
			strcpy(valgrind_trace_path, optarg);
			break;
		default:
			print_help_message();
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char * const * argv) {
	bool verbose_flag = false;
	char valgrind_trace_path[64];
	
	int set_index_bits = 0;
	int associativity = 0;
	int offset_bits = 0;

	parse_clargs(argc, argv, &verbose_flag, &set_index_bits, &associativity, &offset_bits, valgrind_trace_path);	
	// printf("Verbose: %c, s = %d, E = %d, b = %d, trace: %s", verbose_flag ? 'y' : 'n', set_index_bits, associativity, offset_bits, valgrind_trace_path);
	
	cache_t cache = create_cache(1 << set_index_bits, associativity, 1 << offset_bits);
	report_t report = simulate_cache(&cache, valgrind_trace_path, verbose_flag);
	destroy_cache(&cache);

	printSummary(report.hit, report.miss, report.eviction);
	
	return EXIT_SUCCESS;
}
