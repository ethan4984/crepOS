#ifndef NVME_HPP_
#define NVME_HPP_
 
#include <drivers/pci.hpp>
#include <cpu.hpp>
 
namespace nvme {
 
constexpr size_t version_1_0_0_id = (1 << 8 | 0) << 8 | 0;
constexpr size_t version_1_1_0_id = (1 << 8 | 1) << 8 | 0;
constexpr size_t version_1_2_0_id = (1 << 8 | 2) << 8 | 0;
constexpr size_t version_1_2_1_id = (1 << 8 | 2) << 8 | 1;
constexpr size_t version_1_3_0_id = (1 << 8 | 3) << 8 | 0;
constexpr size_t version_1_4_0_id = (1 << 8 | 4) << 8 | 0;

struct [[gnu::packed]] bar_regs {
    uint64_t cap;   
    uint32_t vs;  
    uint32_t intms;
    uint32_t intmc;
    uint32_t cc; 
    uint32_t rsvd1; 
    uint32_t csts;
    uint32_t rsvd2;
    uint32_t aqa; 
    uint64_t asq; 
    uint64_t acq;
};
 
constexpr size_t opcode_del_sq = 0x0;
constexpr size_t opcode_create_sq = 0x1;
constexpr size_t opcode_delete_cq = 0x4;
constexpr size_t opcode_create_cq = 0x5;
constexpr size_t opcode_identify = 0x6;
constexpr size_t opcode_abort = 0x8;
constexpr size_t opcode_set_features = 0x9;
constexpr size_t opcode_get_features = 0xa;
constexpr size_t opcode_namespace_management = 0xd;
constexpr size_t opcode_format_cmd = 0x80;
 
struct [[gnu::packed]] create_completion_queue {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t rsvd1[5];
    uint64_t prp1;
    uint64_t rsvd8;
    uint16_t cqid;
    uint16_t qsize;
    uint16_t cq_flags;
    uint16_t irq_vector;
    uint32_t rsvd12[4];
}; 
 
struct [[gnu::packed]] create_submission_queue {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t rsvd1[5];
    uint64_t prp1;
    uint64_t rsvd8;
    uint16_t sqid;
    uint16_t qsize;
    uint16_t sq_flags;
    uint16_t cqid;
    uint32_t rsvd12[4];
}; 
 
struct [[gnu::packed]] delete_queue {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t rsvd1[9];
    uint16_t qid;
    uint16_t rsvd10;
    uint32_t rsvd11[5];
}; 
 
struct [[gnu::packed]] abort_command {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t rsvd1[9];
    uint16_t sqid;
    uint16_t cid;
    uint32_t rsvd11[5];
}; 
 
struct [[gnu::packed]] features {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t nsid;
    uint64_t rsvd2[2];
    uint64_t prp1;
    uint64_t prp2;
    uint32_t fid;
    uint32_t dword11;
    uint32_t rsvd12[4];
}; 
 
struct [[gnu::packed]] identify {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t nsid;
    uint64_t rsvd2[2];
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cns;
    uint32_t rsvd11[5];
}; 
 
struct [[gnu::packed]] format_command {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t nsid;
    uint64_t rsvd2[4];
    uint32_t cdw10;
    uint32_t rsvd11[5];
}; 

struct [[gnu::packed]] completion {
    uint32_t result;
    uint32_t rsvd;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t command_id;
    uint16_t status;
};

struct [[gnu::packed]] io_rw {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t nsid;
    uint64_t rsvd2;
    uint64_t metadata;
    uint64_t prp1;
    uint64_t prp2;
    uint64_t slba;
    uint16_t length;
    uint16_t control;
    uint32_t dsmgmt;
    uint32_t reftag;
    uint16_t apptag;
    uint16_t appmask;
};
 
struct command {
    union {
        create_completion_queue create_cq;
        create_submission_queue create_sq;
        delete_queue delete_q;
        abort_command abort_cmd;
        features fet;
        identify ident;
        format_command format_cmd;
        io_rw rw;
    };
};

struct [[gnu::packed]] power_state_id {
    uint16_t max_power;
    uint8_t rsvd2;
    uint8_t flags;
    uint32_t entry_lat;
    uint32_t exit_lat;
    uint8_t read_tput;
    uint8_t read_lat;
    uint8_t write_tput;
    uint8_t write_lat;
    uint16_t idle_power;
    uint8_t idle_scale;
    uint8_t rsvd19;
    uint16_t active_power;
    uint8_t active_work_scale;
    uint8_t rsvd23[9];
};

struct [[gnu::packed]] controller_id {
    uint16_t vid;
    uint16_t ssvid;
    char sn[20];
    char mn[40];
    char fr[8];
    uint8_t rab;
    uint8_t ieee[3];
    uint8_t mic;
    uint8_t mdts;
    uint16_t cntlid;
    uint32_t ver;
    uint8_t rsvd84[172];
    uint16_t oacs;
    uint8_t acl;
    uint8_t aerl;
    uint8_t frmw;
    uint8_t lpa;
    uint8_t elpe;
    uint8_t npss;
    uint8_t avscc;
    uint8_t apsta;
    uint16_t wctemp;
    uint16_t cctemp;
    uint8_t rsvd270[242];
    uint8_t sqes;
    uint8_t cqes;
    uint8_t rsvd514[2];
    uint32_t nn;
    uint16_t oncs;
    uint16_t fuses;
    uint8_t fna;
    uint8_t vwc;
    uint16_t awun;
    uint16_t awupf;
    uint8_t nvscc;
    uint8_t rsvd531;
    uint16_t acwu;
    uint8_t rsvd534[2];
    uint32_t sgls;
    uint8_t rsvd540[1508];
    power_state_id psd[32];
    uint8_t vs[1024];
};

struct [[gnu::packed]] lbaf {
    uint16_t ms;
    uint8_t ds;
    uint8_t rp;
};

struct [[gnu::packed]] namespace_id {
    uint64_t nsze;
    uint64_t ncap;
    uint64_t nuse;
    uint8_t nsfeat;
    uint8_t nlbaf;
    uint8_t flbas;
    uint8_t mc;
    uint8_t dpc;
    uint8_t dps;
    uint8_t nmic;
    uint8_t rescap;
    uint8_t fpi;
    uint8_t rsvd33;
    uint16_t nawun;
    uint16_t nawupf;
    uint16_t nacwu;
    uint16_t nabsn;
    uint16_t nabo;
    uint16_t nabspf;
    uint16_t rsvd46;
    uint64_t nvmcap[2];
    uint8_t rsvd64[40];
    uint8_t nguid[16];
    uint8_t eui64[8];
    lbaf lbaf_list[16];
    uint8_t rsvd192[192];
    uint8_t vs[3712];
};

class device;

struct queue {
    queue(device *parent, ssize_t qid);
    queue() = default;
    
    uint16_t send_cmd(command cmd);

    command *submission_queue;
    completion *completion_queue;

    volatile uint32_t *submission_doorbell;
    volatile uint32_t *completion_doorbell;

    size_t qid;
    size_t sq_head;
    size_t sq_tail;
    size_t cq_head;
    size_t cq_tail;
    size_t phase;

    device *parent;
};

class ns {
public:
    ns(namespace_id ns_id, size_t nsid);
    ns() = default;

    int rw_lba(void *buf, size_t start, size_t cnt, bool rw);

    size_t lba_cnt;
    size_t lba_size;
    size_t nsid;
private:
    namespace_id ns_id;
};

struct msd;
 
class device {
public: 
    device(pci::device pci_device);
    device() = default;

    int get_ctrl_id();
    int get_namespace_id(int num);
    int create_io_queue(queue new_queue);

    lib::vector<uint32_t> get_nsid_list();

    friend queue;
    friend msd;
private: 
    pci::device pci_device;
    pci::bar bar;
    volatile bar_regs *registers;
     
    size_t queue_entries;
    size_t max_page_size;
    size_t min_page_size;
    size_t page_size;
    size_t strides;
    size_t qid_cnt;

    queue admin_queue;

    controller_id *ctrl_id;
    namespace_id *ns_id;

    lib::vector<ns> namespace_list;
 
    size_t lock;
};

inline lib::vector<device*> device_list;
 
} 
 
#endif 
