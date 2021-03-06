/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

#ifndef	_SYS_QAT_H
#define	_SYS_QAT_H

typedef enum qat_compress_dir {
	QAT_DECOMPRESS = 0,
	QAT_COMPRESS = 1,
} qat_compress_dir_t;

typedef enum qat_encrypt_dir {
	QAT_DECRYPT = 0,
	QAT_ENCRYPT = 1,
} qat_encrypt_dir_t;


#if defined(_KERNEL) && defined(HAVE_QAT)
#include <sys/zio.h>
#include <sys/crypto/api.h>
#include "cpa.h"
#include "dc/cpa_dc.h"
#include "lac/cpa_cy_sym.h"

/*
 * Timeout - no response from hardware after 0.5 seconds
 */
#define	QAT_TIMEOUT_MS		500

/*
 * The minimal and maximal buffer size, which are not restricted
 * in the QAT hardware, but with the input buffer size between 4KB
 * and 128KB, the hardware can provide the optimal performance.
 */
#define	QAT_MIN_BUF_SIZE	(4*1024)
#define	QAT_MAX_BUF_SIZE	(128*1024)

/*
 * Used for qat kstat.
 */
typedef struct qat_stats {
	/*
	 * Number of jobs submitted to qat compression engine.
	 */
	kstat_named_t comp_requests;
	/*
	 * Total bytes sent to qat compression engine.
	 */
	kstat_named_t comp_total_in_bytes;
	/*
	 * Total bytes output from qat compression engine.
	 */
	kstat_named_t comp_total_out_bytes;
	/*
	 * Number of jobs submitted to qat de-compression engine.
	 */
	kstat_named_t decomp_requests;
	/*
	 * Total bytes sent to qat de-compression engine.
	 */
	kstat_named_t decomp_total_in_bytes;
	/*
	 * Total bytes output from qat de-compression engine.
	 */
	kstat_named_t decomp_total_out_bytes;
	/*
	 * Number of fails in the qat compression / decompression engine.
	 * Note: when qat fail happens, it doesn't mean a critical hardware
	 * issue. Sometimes it is because the output buffer is not big enough.
	 * The compression job will be transfered to gzip software
	 * implementation, so the functionality of ZFS is not impacted.
	 */
	kstat_named_t dc_fails;

	/*
	 * Number of jobs submitted to qat encryption engine.
	 */
	kstat_named_t encrypt_requests;
	/*
	 * Total bytes sent to qat encryption engine.
	 */
	kstat_named_t encrypt_total_in_bytes;
	/*
	 * Total bytes output from qat encryption engine.
	 */
	kstat_named_t encrypt_total_out_bytes;
	/*
	 * Number of jobs submitted to qat decryption engine.
	 */
	kstat_named_t decrypt_requests;
	/*
	 * Total bytes sent to qat decryption engine.
	 */
	kstat_named_t decrypt_total_in_bytes;
	/*
	 * Total bytes output from qat decryption engine.
	 */
	kstat_named_t decrypt_total_out_bytes;
	/*
	 * Number of fails in the qat encryption / decryption engine.
	 * Note: when qat fail happens, it doesn't mean a critical hardware
	 * issue. Sometimes it is because the output buffer is not big enough.
	 * The encryption job will be transfered to the software implementation,
	 * so the functionality of ZFS is not impacted.
	 */
	kstat_named_t crypt_fails;
} qat_stats_t;

#define	QAT_STAT_INCR(stat, val) \
	atomic_add_64(&qat_stats.stat.value.ui64, (val))
#define	QAT_STAT_BUMP(stat) \
	QAT_STAT_INCR(stat, 1)

extern qat_stats_t qat_stats;
extern int zfs_qat_disable;

/* inlined for performance */
static inline struct page *
qat_mem_to_page(void *addr)
{
	if (!is_vmalloc_addr(addr))
		return (virt_to_page(addr));

	return (vmalloc_to_page(addr));
}

CpaStatus qat_mem_alloc_contig(void **pp_mem_addr, Cpa32U size_bytes);
void qat_mem_free_contig(void **pp_mem_addr);
#define	QAT_PHYS_CONTIG_ALLOC(pp_mem_addr, size_bytes)	\
	qat_mem_alloc_contig((void *)(pp_mem_addr), (size_bytes))
#define	QAT_PHYS_CONTIG_FREE(p_mem_addr)	\
	qat_mem_free_contig((void *)&(p_mem_addr))

extern int qat_dc_init(void);
extern void qat_dc_fini(void);
extern int qat_crypt_init(void);
extern void qat_crypt_fini(void);
extern int qat_init(void);
extern void qat_fini(void);

extern boolean_t qat_dc_use_accel(size_t s_len);
extern boolean_t qat_crypt_use_accel(size_t s_len);
extern int qat_compress(qat_compress_dir_t dir, char *src, int src_len,
    char *dst, int dst_len, size_t *c_len);
extern int qat_crypt(qat_encrypt_dir_t dir, uint8_t *src_buf, uint8_t *dst_buf,
    uint8_t *aad_buf, uint32_t aad_len, uint8_t *iv_buf, uint8_t *digest_buf,
    crypto_key_t *key, uint64_t crypt, uint32_t enc_len);
#else
#define	CPA_STATUS_SUCCESS					0
#define	qat_init()
#define	qat_fini()
#define	qat_dc_use_accel(s_len)					0
#define	qat_crypt_use_accel(s_len)				0
#define	qat_compress(dir, s, sl, d, dl, cl)			0
#define	qat_crypt(dir, s, d, a, al, i, db, k, c, el)		0
#endif

#endif /* _SYS_QAT_H */
