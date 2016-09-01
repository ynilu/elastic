#Elastic Optical Network 程式說明

## Introduction

程式主要分成下列11個檔案:
- auxiliary.hpp
- auxiliary.cpp
- graph.hpp
- graph.cpp
- light_path.hpp
- light_path.cpp
- spectrum.hpp
- spectrum.cpp
- traffic.hpp
- traffic.cpp
- simulator.cpp

其中 .hpp 檔是定義了個別 class 的 header 檔
.cpp 檔則是對應的 function definition

simulator.cpp 則是整個程式的中樞，利用其他檔案提供的物件來進行模擬
下面會對每個檔案分別作說明

## auxiliary.hpp

宣告所有 auxiliary graph 需要的物件(ex. auxiliary node, auxiliary link)

### Auxiliary_info

一個 struct 用來存放所有要帶進輔助圖的變數(ex. number of physical nodes, number of slots per physical link)

### Aux_node

auxiliary node 的 class, 使用了兩個 doubly linked list 來記錄所有連入及連出的 auxiliary link

- data
  - `Aux_node_type`: enumeration of types of auxiliary node
  - `type`: the type of auxiliary node (ex. adding node dropping node)
  - `phy_id`: 這個 auxiliary node 所屬的 physical node id
  - `first_in`: a pointer which point to the head of the incoming doubly linked list
  - `last_in`: a pointer which point to the tail of the incoming doubly linked list
  - `first_out`: a pointer which point to the head of the outgoing doubly linked list
  - `last_out`: a pointer which point to the tail of the outgoing doubly linked list
- function
  - `Aux_node()`: constructor of Aux_node
  - `~Aux_node()`: destructor of Aux_node


### Aux_link

auxiliary graph

auxiliary link 的 class, 也是上述 doubly linked list 中的 element, 比較特別地方是一個 element 會存在在兩個 linked list 中(一個在 Aux_link 的 source Aux_node 的 outgoing list, 另一個在 Aux_link 的 destination Aux_node 的 incoming list)

- data
  - `Aux_link_type`: enumeration of types of auxiliary link
  - `type`: the type of auxiliary link (ex. adding link dropping link)
  - `light_path`: a pointer, 指向這個 auxiliary link 所屬的 light path
  - `weight`: 這個 auxiliary link 的weight(cost)
  - `from`: a pointer, 指向這個 auxiliary link 的 source Aux_node
  - `to`: a pointer, 指向這個 auxiliary link 的 destination Aux_node
  - `prev_same_from`: a pointer, 指向同個 source Aux_node 的 doubly linked list 中的上一個 Aux_link
  - `next_same_from`: a pointer, 指向同個 source Aux_node 的 doubly linked list 中的下一個 Aux_link
  - `prev_same_to`: a pointer, 指向同個 destination Aux_node 的 doubly linked list 中的上一個 Aux_link
  - `next_same_to`: a pointer, 指向同個 destination Aux_node 的 doubly linked list 中的下一個 Aux_link
- function
  - `Aux_link()`: constructor of Aux_link
  - `~Aux_link()`: destructor of Aux_link

### Aux_graph

整個 auxiliary graph 的 class

- data
  - `adding_node_list`: vector of all adding nodes on the auxiliary graph
  - `dropping_node_list`: vector of all dropping nodes on the auxiliary graph
  - `OFDM_WOB_virtual_transmitting_node_list`: vector of all transmitting nodes in OFDM_WOB layer on the auxiliary graph
  - `OFDM_WOB_virtual_receiving_node_list`: vector of all receiving nodes in OFDM_WOB layer on the auxiliary graph
  - `OFDM_WB_virtual_transmitting_node_list`: vector of all transmitting nodes in OFDM_WB layer on the auxiliary graph
  - `OFDM_WB_virtual_receiving_node_list`: vector of all receiving nodes in OFDM_WB layer on the auxiliary graph
  - `OFDM_virtual_transmitting_node_list`: vector of all transmitting nodes in OFDM layer on the auxiliary graph
  - `OFDM_virtual_receiving_node_list`: vector of all receiving nodes in OFDM layer on the auxiliary graph
  - `OTDM_virtual_transmitting_node_list`: vector of all transmitting nodes in OTDM layer on the auxiliary graph
  - `OTDM_virtual_receiving_node_list`: vector of all receiving nodes in OTDM layer on the auxiliary graph
- function
  - `get_adding_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 adding node
  - `get_dropping_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 dropping node
  - `get_OFDM_WOB_virtual_transmitting_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM_WOB layer 的 transmitting node
  - `get_OFDM_WOB_virtual_receiving_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM_WOB layer 的 receiving node
  - `get_OFDM_WB_virtual_transmitting_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM_WB layer 的 transmitting node
  - `get_OFDM_WB_virtual_receiving_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM_WB layer 的 receiving node
  - `get_OFDM_virtual_transmitting_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM layer 的 transmitting node
  - `get_OFDM_virtual_receiving_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM layer 的 receiving node
  - `get_OTDM_virtual_transmitting_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OTDM layer 的 transmitting node
  - `get_OTDM_virtual_receiving_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OTDM layer 的 receiving node
  - `create_aux_node()`: 建立一個新的 auxiliary node
  - `create_aux_link()`: 建立一條新的 auxiliary link 並加入新的 link 到對應的 linked list 中
  - `Aux_graph()`: constructor of Aux_graph
  - `~Aux_graph()`: destructor of Aux_graph

## auxiliary.cpp

定義所有 auxiliary.hpp 中宣告的 functions

```c++
Aux_node::Aux_node(int phy_id, Aux_node_type type)
```
初始化 Aux_node 的 data

```c++
Aux_node::~Aux_node()
```
在 Aux_node 被從輔助圖上移除的時候, 要先把要移除的 Aux_node 的所有 outgoing links and incoming links 移除 

```c++
Aux_link::Aux_link(Aux_node* from, Aux_node* to, double weight, Aux_link_type type)
```
初始化 Aux_link 的 data, 並把這個新建立的 Aux_link 加入 `from` 的 outgoing linked list 以及 `to` 的 incoming linked list

```c++
Aux_link::~Aux_link()
```
在 Aux_link 被從輔助圖上移除的時候, 必須把要移除的 Aux_link 從其屬於的 outgoing linked list 以及 incoming linked list 中移除

```c++
Aux_graph::Aux_graph(Auxiliary_info& a_info)
```
初始化所有的 auxiliary node list, 並建立屬於每個 physical node 在 OTDM, OFDM, OFDM_WB, OFDM_WOB layer 的 transmitting and receiving node 以及在 grooming layer 的 adding node and dropping node, 同時建立對應的 grooming link, adding link, dropping link

```c++
Aux_graph::~Aux_graph()
```
在 Aux_graph 被 delete 時, delete 所有的 node list 中的 Aux_node, 同時也會在 Aux_node 的 destructor delete 相連的 Aux_link

```c++
Aux_node* Aux_graph::create_aux_node(int phy_id, Aux_node::Aux_node_type type)
```
new 一個新的 Aux_node

```c++
Aux_link* Aux_graph::create_aux_link(Aux_node* from, Aux_node* to, double weight, Aux_link::Aux_link_type type)
```
new 一個新的 Aux_link

```c++
Aux_node* Aux_graph::get_adding_node(int phy_id)
```
從 adding_node_list 中取出 phy_id 這個 physical node 的 adding node

```c++
Aux_node* Aux_graph::get_dropping_node(int phy_id)
```
從 dropping_node_list 中取出 phy_id 這個 physical node 的 dropping_node

```c++
Aux_node* Aux_graph::get_OFDM_WOB_virtual_transmitting_node(int phy_id)
```
從 OFDM_WOB_virtual_transmitting_node_list 中取出 phy_id 這個 physical node 的 OFDM_WOB_virtual_transmitting_node

```c++
Aux_node* Aux_graph::get_OFDM_WOB_virtual_receiving_node(int phy_id)
```
從 OFDM_WOB_virtual_receiving_node_list 中取出 phy_id 這個 physical node 的 OFDM_WOB_virtual_receiving_node

```c++
Aux_node* Aux_graph::get_OFDM_WB_virtual_transmitting_node(int phy_id)
```
從 OFDM_WB_virtual_transmitting_node_list 中取出 phy_id 這個 physical node 的 OFDM_WB_virtual_transmitting_node

```c++
Aux_node* Aux_graph::get_OFDM_WB_virtual_receiving_node(int phy_id)
```
從 OFDM_WB_virtual_receiving_node_list 中取出 phy_id 這個 physical node 的 OFDM_WB_virtual_receiving_node

```c++
Aux_node* Aux_graph::get_OFDM_virtual_transmitting_node(int phy_id)
```
從 OFDM_virtual_transmitting_node_list 中取出 phy_id 這個 physical node 的 OFDM_virtual_transmitting_node

```c++
Aux_node* Aux_graph::get_OFDM_virtual_receiving_node(int phy_id)
```
從 OFDM_virtual_receiving_node_list 中取出 phy_id 這個 physical node 的 OFDM_virtual_receiving_node

```c++
Aux_node* Aux_graph::get_OTDM_virtual_transmitting_node(int phy_id)
```
從 OTDM_virtual_transmitting_node_list 中取出 phy_id 這個 physical node 的 OTDM_virtual_transmitting_node

```c++
Aux_node* Aux_graph::get_OTDM_virtual_receiving_node(int phy_id)
```
從 OTDM_virtual_receiving_node_list 中取出 phy_id 這個 physical node 的 OTDM_virtual_receiving_node


## graph.hpp

宣告所有 physical graph 需要的物件(ex. physical node, physical link)

```c++
const double LAUNCH_OSNR = 24.5;
const double LAUNCH_POWER_DENSITY = 25;
const double ATTENUATION_PARA = 0.2;
const double NOISE_PENALTY_FACTOR = 1;
const double AMPLIFIER_NOISE_FIGURE = 6;
const double RESIDUAL_DISPERSION_RATIO = 1;
const double FIBER_NONLINEARITY_COEFFICIENT = 1.22;
const double VELOCITY_DISPERSION_PARA = 16;
const double TOTAL_FIBER_BANDWIDTH = 4;
const double LIGHT_FREQUENCY = 196.1e3;
const double PLANCK = 6.625e-34;
const double PI = 3.1416;
const double QPSK_OSNR = 17.5;
const double OSNR[5] = {-1, -1, 0, 2, 4};
const int SPAN_LEN = 82;
```
計算light path reach 相關係數

```c++
typedef std::vector<int> Path;
```
以 vector<int> 來表示一條 physical path, 記錄 path 經過的 physical node 的 id

```c++
typedef std::pair<int, int> Node_pair;
```
以 vector<int ,int> 來表示一對 physical node pair

```c++
typedef std::vector<int> Parents;
```
在尋找 shortest path 時用來記錄一個 physical node 在 shortest path 上的所有 parents (可能有多個)

```c++
typedef std::map< Node_pair, std::list<CandidatePath> > Candidate_path_list;
```
記錄 node pair 所有可能建立 candidate light path 的資料結構 CandidatePath 在下面會有較詳細的說明

```c++
typedef std::vector<Phy_node> Phy_node_list;
```
所有 Physical node 的 vector

```c++
typedef std::map< Node_pair, Phy_link > Phy_link_list;
```
所有 Physical link 的 map

### Graph_info

一個 struct 用來存放所有要帶進 physical graph 的變數(ex. graph file, number of slots per physical link)

### Transceiver

OTDM transmitter, OTDM receiver, OFDM sub-transmitter OFDM sub-receiver 的 class

- data
  - `spectrum`: 這個物件所佔用的 spectrum 在 spectrum.hpp 中會再詳細說明
- function
  - `Transceiver()`: constructor of Transceiver
  - `~Transceiver()`: destructor of Transceiver

### OFDMTransceiver

OFDM transmitter, OFDM receiver 的 class

- data
  - `in_used`: 用來記錄這個 OFDM transceiver 是否有被使用
  - `num_available_sub_transceiver`: 記錄這個 OFDM transceiver 可用的 sub_transceiver 數量
  - `light_path`: 記錄使用這個 OFDM transceiver 的 light path 集合
  - `sub_transceiver`: 這個物件所有 OFDM sub-transceiver 集合
  - `spectrum`: 這個物件所佔用的 spectrum 在 spectrum.hpp 中會再詳細說明
- function
  - `OFDMTransceiver()`: constructor of OFDMTransceiver
  - `~OFDMTransceiver()`: destructor of OFDMTransceiver
  
### Phy_node

Physical node 的 class

- data
  - `degree`: 用來記錄這個 physical node 的 degree 數, 也就是他有多少個 neighbors
  - `neighbor`: 用來記錄這個 physical node 的 neighbor 集合, 存放 neighbor 的 physical node id
  - `num_available_transmitter`: 記錄使用這個 physical node 可用的 OTDM transmitter 數量
  - `num_available_OFDM_transmitter`: 記錄使用這個 physical node 可用的 OFDM transmitter 數量
  - `num_available_receiver`: 記錄使用這個 physical node 可用的 OTDM receiver 數量
  - `num_available_OFDM_receiver`: 記錄使用這個 physical node 可用的 OFDM receiver 數量
  - `transmitter`: 這個物件所有 OTDM transmitter 集合
  - `receiver`: 這個物件所有 OTDM receiver 集合
  - `OFDMtransmitter`: 這個物件所有 OFDM transmitter 集合
  - `OFDMreceiver`: 這個物件所有 OFDM receiver 集合
- function
  - `Phy_node()`: constructor of Phy_node
  - `~Phy_node()`: destructor of Phy_node
  
### Phy_link

Physical link 的 class

- data
  - `source`: 這個 physical link 的 source node 的 physical node id
  - `destination`: 這個 physical link 的 destination node 的 physical node id
  - `distance`: 這個 physical link source 到 destination 的實際距離
  - `slot`: 這個 physical link 所有的 slot 使用狀態 (-1 -> free; 0 -> guardband; 1 -> occupied)
- function
  - `Phy_link()`: constructor of Phy_link
  - `~Phy_link()`: destructor of Phy_link

### CandidatePath

candidate light path 的 class

- data
  - `path`: 這個 candidate path 的 physical node path
  - `modulation_level`: 這個 candidate path 最所可以使用最高的 modulation level
- function
  - `Phy_graph()`: constructor of Phy_graph
  - `~Phy_graph()`: destructor of Phy_graph
  
### Phy_graph

physical graph 的 class

- data
  - `node_list`: 所有的 physical node
  - `link_list`: 所有的 physical link
  - `path_list`: 所有的 candidate path
- function
  - `read_network_file()`: 從指定的檔案中讀取 network 的架構, 並存到對應的資料結構中
  - `assign_transceivers()`: 初始化所有 physical node 的 transceiver 以及可用的 transceiver 數量
  - `find_candidate_path()`: 對所有的 node pair 執行 `BFS_find_path()` 來找 shortest path
  - `BFS_find_path()`: 對 physical graph 中的一組 node pair 跑 BFS 用來尋找所有的 shortest path
  - `DFS_back_trace()`: 對 `BFS_find_path()` 的結果, 做 DFS 來列出所有的 shortest path
  - `get_reach()`: 計算給定路徑所能使用最高的 modulation level 如果這條路徑過長無法使用任何一種 modulation 建立 light path 則回傳 `-1`
  - `d_osnr()`: 用來計算 signal 經過一個 physical node 所減少的 OSNR
  - `modlev()`: 計算給定路徑距離所能使用最高的 modulation level 如果這條路徑過長無法使用任何一種 modulation 建立 light path 則回傳 `-1`
  - `get_path_list()`: 取得一個 reference, 指向給定 node pair 的所有 candidate path
  - `CandidatePath()`: constructor of CandidatePath
  - `~CandidatePath()`: destructor of CandidatePath

## graph.cpp

定義所有 graph.hpp 中宣告的 functions
  
```c++
double Phy_graph::d_osnr(int span_num, int span_len)
```
用來計算 signal 經過一個 physical node 所減少的 OSNR

```c++
int Phy_graph::modlev(vector<int>dis_vec)
```
計算給定路徑距離所能使用最高的 modulation level 如果這條路徑過長無法使用任何一種 modulation 建立 light path 則回傳 `-1`

```c++
int Phy_graph::get_reach(vector<int> path)
```
取出 `path` 的 distance 資訊, 呼叫 `modlev()` 進行 reach 計算

```c++
Phy_graph::Phy_graph(Graph_info &g_info)
```
- call `read_network_file()`
- call `assign_transceivers()`
- call `find_candidate_path()`

```c++
void Phy_graph::assign_transceivers(int num_OTDM_transceiver, int num_OFDM_transceiver, int transceiver_connection_limit)
```
對所有的 physical node 的 OTDM transmitter, OTDM receiver, OFDM transmitter, OFDM receiver 進行初始化

```c++
void Phy_graph::find_candidate_path()
```
對每一對 node pair 呼叫 `BFS_find_path()` 來尋找 candidate path

```c++
void Phy_graph::BFS_find_path(int source, int destination)
```
從 `source` 開始做 BFS 記錄每個 physical node 是被哪些在 BFS 上一層中的 physical node 發現的, 並記錄在 `parents` 中, 在呼叫 `DFS_back_trace()` 來列出所有的 candidate path

```c++
void Phy_graph::DFS_back_trace(int current_node, vector<Parents>& parents, list<CandidatePath>& path_set, CandidatePath& path)
```
一個 recursive function, 每次進來這個 function 先將 `current_node` push_back path 這個暫存的路徑, 如果 `parents[current_node]` 是空的代表 DFS 找到 source node 了, 這個時候把 `path` 複製成 `new_path` 並 reverse 成正確的方向(從 source node 到 destination node) 再呼叫 `get_reach()` 來取得最好的 modulation level 如果這個 `new_path` 有 modulation 可以用, 把 path 加入對應的 candidate path set 中, 處理完找到 source node 的情況之後再以 `parents[current_node]` 為下次的 `current_node` 做 recursive call, 最後把 `current_node` 從 `path` 中移除

```c++
Phy_node& Phy_graph::get_node(int id)
```
取的指定 physical node id 的 `Phy_node` object

```c++
Phy_link& Phy_graph::get_link(int source, int destination)
```
取的指定 node pair 的 `Phy_link` object

```c++
list<CandidatePath>& Phy_graph::get_path_list(int source, int destination)
```
取的指定 node pair 的 candidate path set

```c++
void Phy_graph::read_network_file(char* graph_file, int num_slots)
```
從指定檔案(graph_file) 中讀出 number of physical nodes, physical links 等資訊, 並建立對應的 physical node 及 physical link

## light_path.hpp

宣告 light path class

```c++
typedef std::vector<int> Path;
```
以 vector<int> 來表示一條 physical path, 記錄 path 經過的 physical node 的 id

### LightPath

LightPath 的 class

- data
  - `LightPath_type`: enumeration of types of auxiliary node
  - `type`: the type of light path (ex. adding node dropping node)
  - `requests`: 這個 light path 所傳輸的 requests 集合
  - `modulation_level`: 這個 light path 的 modulation level
  - `available_bitrate`: 這個 light path 還可以 grooming 新 requests 的 bitrate
  - `weight`: 這個 light path 在 輔助圖上的 weight
  - `spectrum`: 這個 light path 所使用的 spectrum
  - `p_path`: 這個 light path 的 physical node path
  - `transmitting_node_list`: 代表這個 light path 的 auxiliary transmitting node set
  - `receiving_node_list`: 代表這個 light path 的 auxiliary receiving node set
  - `aux_link_list`: 代表這個 light path 的 auxiliary link set
  - `transmitter_index`: 用來記錄這個 light path 的 transmitter 使用情況
    - `transmitter_index[i] == -1` 代表這個 light path 在 p_path[i] 這個 physical node 上沒有使用 transmitter
    - `transmitter_index[i] == id` 代表這個 light path 在 p_path[i] 這個 physical node 上使用了 transmitter[id] 這個 transmitter
  - `receiver_index`: 用來記錄這個 light path 的 receiver 使用情況
    - `receiver_index[i] == -1` 代表這個 light path 在 p_path[i] 這個 physical node 上沒有使用 receiver
    - `receiver_index[i] == id` 代表這個 light path 在 p_path[i] 這個 physical node 上使用了 receiver[id] 這個 receiver
- function
  - `LightPath()`: constructor of LightPath
  - `~LightPath()`: destructor of LightPath

## light_path.cpp

定義所有 light_path.hpp 中宣告的 functions

## spectrum.hpp

宣告 spectrum class

### Spectrum

Spectrum 的 class

- data
  - `slot_st`: 一個 spectrum 的起始 slot id (id 比較小的為起始 id)
  - `slot_ed`: 一個 spectrum 的結束 slot id (id 比較大的為結束 id)
  - `weight`: 這個 spectrum 的 weight, 用來找出最好的 spectrum 時會拿出來比較
- function
  - `Spectrum()`: constructor of Spectrum
  - `~Spectrum()`: destructor of Spectrum

## spectrum.cpp

定義所有 spectrum.hpp 中宣告的 functions

## traffic.hpp

宣告所有跟 requests 相關的 class

### Traffic_info

一個 struct 用來存放所有要帶進 traffic 的變數(ex. traffic file, number of physical nodes, time seed, reqeust bitrate share)

### Event

request event 的 class

- data
  - `event_type`: enumeration of types of request event
  - `type`: the type of request event (ex. arrival, departure)
  - `request_id`: 這個 event 所屬的 request id
  - `source`: 這個 event (request) 的 source physical node
  - `destination`: 這個 event (request) 的 destination physical nodes (為了方便以後改成 multicast 現在只會放一個 destination)
  - `num_dest`: destination node 的數量 (為了方便以後改成 multicast 現在只會為 1)
  - `bandwidth`: 這個 event (request) 所需要的 bitrate
  - `arrival_time`: 這個 event (request) 的抵達時間
  - `holding_time`: 這個 event (request) 在網路上停留的時間
- function
  - `operator <()`: 用來排序所有的 event 所需的 operator `<`
  - `Event()`: constructor of Event
  - `~Event()`: destructor of Event

### Traffic

所有 traffic 的 class

- data
  - `total_dest_count`: 用來記錄各個 node pair 的 request 數
  - `traffic_matrix`: 記錄 request 從一個 physical node 到另一個 physical node 的機率
  - `source_matrix`: 記錄一個 physical node 成為 request source 的機率
  - `num_dest_matrix`: 記錄一個 source node 最多可以有多少個 destinations
  - `event_list`: 存放所有的 request events 的 list
  - `num_nodes`: 網路上所有 physical node 的數量
  - `num_requests`: request 總數
  - `total_OCx_share`: 所有 OCx_share 的加總, 用來計算各種 OCx request 出現的機率
  - `OC1_ratio`: OC1 的 request 出現的機率
  - `OC3_ratio`: OC3 的 request 出現的機率
  - `OC9_ratio`: OC9 的 request 出現的機率
  - `OC12_ratio`: OC12 的 request 出現的機率
  - `OC18_ratio`: OC18 的 request 出現的機率
  - `OC24_ratio`: OC24 的 request 出現的機率
  - `OC36_ratio`: OC36 的 request 出現的機率
  - `OC48_ratio`: OC48 的 request 出現的機率
  - `OC192_ratio`: OC192 的 request 出現的機率
  - `OC768_ratio`: OC768 的 request 出現的機率
  - `OC3072_ratio`: OC3072 的 request 出現的機率
  - `num_OC1_request`: OC1 request 的數量
  - `num_OC3_request`: OC3 request 的數量
  - `num_OC9_request`: OC9 request 的數量
  - `num_OC12_request`: OC12 request 的數量
  - `num_OC18_request`: OC18 request 的數量
  - `num_OC24_request`: OC24 request 的數量
  - `num_OC36_request`: OC36 request 的數量
  - `num_OC48_request`: OC48 request 的數量
  - `num_OC192_request`: OC192 request 的數量
  - `num_OC768_request`: OC768 request 的數量
  - `num_OC3072_request`: OC3072 request 的數量
  - `traffic_lambda`: 模擬模型的 lambda
  - `traffic_mu`: 模擬模型的 mu
  - `unicast_percentage`: 產生 unicast request 的機率(現在都是設 1, 所有的 request 都是 unicast)
  - `aTime_seed`: arrival time seed
  - `hTime_seed`: holding time seed
  - `s_seed`: source node seed
  - `d_seed`: destination node seed
  - `numD_seed`: number of destination seed
  - `b_seed`: bandwidth seed
- function
  - `read_source_file()`: 從指定的檔案中讀出每個 node 成為 source 的機率, 以及最多能有多少的 destinations
  - `read_traffic_file()`: 從指定的檔案中讀出每一個 source node 到每一個 destination node 的機率
  - `generate_traffic()`: 一次產生所有 request 的 arrival event and departure event 並依時間排序
  - `empty()`: 查看是否所有的 event 都已經被處理, 也就是 `event_list` 是否為 empty
  - `next_event()`: 從 `event_list` 取出最前面的 event, 並將這個 event 從 `event_list` 中移除
  - `delete_event()`: 當 request 被 block 時所使用, 從 `event_list` 中移除給定 request id 的 departure event
  - `generate_num_dest()`: 隨機產生 number of destination (目前都會是 1, 因為 unicast_percentage 是 1)
  - `generate_source()`: 隨機產生 source node
  - `generate_destination()`: 隨機產生 destination node
  - `generate_bandwidth()`: 隨機產生 request 的 bandwidth
  - `random_number()`: 產生一個 0 到 1 之間的數
  - `get_interarrival_time()`: 隨機產生 event 的 interarrival time
  - `nextrand()`: 產生一個隨機數, 用來產生 `random_number()`
  - `Traffic()`: constructor of Traffic
  - `~Traffic()`: destructor of Traffic

## traffic.cpp

定義所有 traffic.hpp 中宣告的 functions

```c++
Traffic::Traffic(Traffic_info& t_info)
```
- 把所有的 OCx_share 加總算出 `total_OCx_share`, 用 OCx_share 除以 `total_OCx_share` 算出各個 OCx_ratio
- 初始化各個變數
- call `read_source_file()`
- call `read_traffic_file()`
- call `generate_traffic()`

```c++
void Traffic::read_source_file(char* source_file)
```
從 source_file 中讀出每個 node 成為 source 的機率, 以及最多能有多少的 destinations

```c++
void Traffic::read_traffic_file(char* traffic_file)
```
從檔案 traffic_file 中讀出每一個 source node 到每一個 destination node 的機率

```c++
bool Traffic::empty()
```
看 `event_list` 是否為空的

```c++
Event Traffic::next_event()
```
從 `event_list` 取出最前面的 event, 並將這個 event 從 `event_list` 中移除

```c++
void Traffic::delete_event(int request_id)
```
掃過整個 `event_list` 找 event type 為 departure 且 request id 為指定 request id 的 event 並移除之

```c++
void Traffic::generate_traffic()
```
一次產生一個 request, 一個 request 會有兩個 event 一個是這個 request arrival, 一個是這個 request departure, 使用 `generate_source()` 決定 request 的 source node, 使用 `generate_destination()` 決定 request 的 destination node, 使用 `generate_bandwidth()` 決定 request 的 bandwidth(所需要的 bitrate), 使用 `get_interarrival_time()` 來決定兩個 request arrival 間的間隔, 在使用一次 `get_interarrival_time()` 來決定 reqeust 的 holding time, 也就是 request 在網路上接受服務的時間, 把每一個 request 的兩種 event 都加入 `event_list` 中, 並在所有的 request 都產生完後, 對 `event_list` 進行排序

```c++
int Traffic::generate_num_dest(int max_num_dest)
```
使用 `random_number()` 產生一個隨機數, 如果產生出來的數字小於等於 `unicast_percentage`, return 1, 否則使用 `random_number()` 再決定有多少個 destinations (範圍是 2 到 `max_num_dest`)

```c++
int Traffic::generate_source()
```
使用 `random_number()` 產生一個隨機數, 看產生數字落在的範圍決定使用哪個 node 當 source

```c++
int Traffic::generate_destination(int source)
```
使用 `random_number()` 產生一個隨機數, 看產生數字落在的範圍決定使用哪個 node 當 destination

```c++
int Traffic::generate_bandwidth()
```
使用 `random_number()` 產生一個隨機數, 看產生數字落在的範圍決定使用哪個 OCx 當 bandwidth

```c++
bool Event::operator <(const Event& a) const
```
以 event 的 arrival time 為比較的根據

```c++
float Traffic::random_number( int seed )
```
產生一個 0 到 1 之間的數

```c++
double Traffic::get_interarrival_time( float mean, int seed )
```
隨機產生 event 的 interarrival time

```c++
long long Traffic::nextrand( long long& seed )
```
產生一個隨機數, 用來產生 `random_number()`

## simulator.cpp

整個程式的 main function

### variables
- `start_clk`: simulation 開始時間 in clock
- `start_clk_finding`: 每一次 bellmanford 開始前的時間 in clock
- `clk_finding`: 花在 bellmanford 上的總時間 in second
- `start_clk_construction`: 每一次 construct auxiliary graph 開始前的時間 in clock 
- `clk_construction`: 花在 construct auxiliary graph 上的總時間 in second
- `start_clk_parsing`: 每一次做 path parsing 開始前的時間 in clock
- `clk_parsing`: 花在 path parsing 上的總時間 in second
- `hop_limit`: hop 數限制
- `unicast_percentage`: unicast request 的比例 (0 ~ 1)
- `num_requests`: total number of requests
- `num_slots`: 一個 physical link 上的 frequency slot 數
- `num_nodes`: total number of physical nodes
- `traffic_lambda`: 模擬模型的 lambda
- `traffic_mu`: 模擬模型的 mu
- `num_transceiver`: 一個 physical node 每多一個 neighbor 所給定的 transceiver 數 (OTDM, OFDM 各分一半)
- `num_OTDM_transceiver`: 一個 physical node 每多一個 neighbor 所給定的 OTDM transceiver 數
- `num_OFDM_transceiver`: 一個 physical node 每多一個 neighbor 所給定的 OFDM transceiver 數
- `slot_capacity`: 一個 slot 的 capacity
- `transceiver_slot_limit`: 一個 transceiver 最多能控制的 slot 數
- `transceiver_connection_limit`: 一個 transceiver 最多能建立的 connection 數
- `num_guardband_slot`: guard band 所需要的 slot 數
- `enable_OTDM`: 是否使用 OTDM light path, 0 -> OFDM, 1 -> OFDM + OTDM
- `OTDM_threshold`: 決定是否要傾向使用 OTDM light path 的 threshold, 一個 light path 的剩餘 bandwidth 量如果大於這個 threshold 則傾向使用 OTDM
- `OC1_share`: OC1 request 所佔的份數
- `OC3_share`: OC3 request 所佔的份數
- `OC9_share`: OC9 request 所佔的份數
- `OC12_share`: OC12 request 所佔的份數
- `OC18_share`: OC18 request 所佔的份數
- `OC24_share`: OC24 request 所佔的份數
- `OC36_share`: OC36 request 所佔的份數
- `OC48_share`: OC48 request 所佔的份數
- `OC192_share`: OC192 request 所佔的份數
- `OC768_share`: OC768 request 所佔的份數
- `OC3072_share`: OC3072 request 所佔的份數
- `aTime_seed`: arrival time seed
- `hTime_seed`: holding time seed
- `s_seed`: source node seed
- `d_seed`: destination node seed
- `numD_seed`: number of destination seed
- `b_seed`: bandwidth seed
- `accepted_requests`: total number of accepted requests
- `blocked_requests`: total number of blocked requests
- `blocked_bandwidth`: total bandwidth of blocked requests
- `num_OEO`: total number of OEO
- `num_OFDM_lightpath_use`: 使用 OFDM light path 的次數
- `num_OTDM_lightpath_use`: 使用 OTDM light path 的次數
- `total_bandwidth`: total bandwidth of all requests
- `eps`: 一個用來調整 weight 的變數
- `transceiver_weight`: OTDM virtual adding link weight
- `used_transceiver_weight`: OTDM adding link weight
- `OFDM_transceiver_weight`: OFDM virtual adding link weight
- `used_OFDM_transceiver_weight`: OFDM adding link weight
- `OEO_weight`: grooming link weight
- `reserved_coefficent`: 計算 spectrum weight 裡, 考慮 reserved 的係數
- `cut_coeffcient`: 計算 spectrum weight 裡, 考慮 cut 的係數
- `align_coeffcient`: 計算 spectrum weight 裡, 考慮 align 的係數
- `candidate_light_path_list`: 存放所有的 candidate light path
- `exist_OTDM_light_path_list`: 存放所有已被建立的 OTDM light path
- `exist_OFDM_light_path_list`: 存放所有已被建立的 OFDM light path
- `request2lightpath`: 一個資料結構, 用來查詢 request 是被 assign 到哪幾條 light path
- `graph_file`: graph file name
- `source_file`: source file name
- `traffic_file`: traffic file name

### functions

```c++
int main(int argc, char *argv[])
```
- 讀取 argv 並 assign 值到對應的變數
- 建立 `Phy_graph` object 建立 physical graph
- 建立 `Traffic` object 產生所有的 requests
- 建立 `Aux_graph` object 產生所有的初始的 auxiliary graph
- 依序處理所有的 events
  - arrival event
    - call `construct_candidate_path()` and `construct_exist_path()` 建立對應這個 request 的輔助圖
    - call `BellmanFordSP()` 找輔助圖上最低 cost 的 path
    - 根據 `BellmanFordSP()` 的結果決定是否執行 `path_paring()` 找出並建立最好的那條 light path
    - call `reset_auxiliary_graph()` 還原輔助圖到最初建立的狀況, 以便下一輪使用
  - departure event
    - 根據 `request2lightpath` 移除 exist light path 上所有的 departure request
    - 如果有 light path 上沒有任何的 request, 則移除這條 light path 並釋放資源
    - 如果 light path 還有 request, 則更新 light path 的 `available_bitrate`
- call `print_result()` output 所有統計的結果到檔案
 
```c++
void construct_exist_path(Event& event, Aux_graph& a_graph)
```
對所有 exist OTDM light path 建立輔助圖上對應的 node and link // TODO may be more detail

```c++
void construct_candidate_path(Event& event, Phy_graph& p_graph, Aux_graph& a_graph)
```
對每一對 physical node pair 找出最好的 light path 並建立對應的 auxiliary candidate link
- 如果 `enable_OTDM` 不為 0, call `best_OTDM_light_path()` 找出最好的 OTDM light path 並建立 candidate link
- call `best_OFDM_light_path()` 找出最好的 OFDM light path 並建立 candidate link
- call `best_OFDM_WB_light_path()` 找出最好的 OFDM WB light path 並建立 candidate link
- call `best_OFDM_WOB_light_path()` 找出最好的 OFDM WOB light path 並建立 candidate link

```c++
void reset_auxiliary_graph()
```
還原輔助圖到最初建立的狀況, 以便下一輪使用
- 移除所有屬於 exist OTDM light path 的 transmitting node 以及 receiving node, 這個動作同時會移除與這些 node 相連的 auxiliary link
- 移除所有屬於 candidate light path 的 candidate link
- 清空 candidate light path list

```c++
void build_candidate_link(Aux_graph& a_graph, LightPath* lpath)
```
- 根據傳入的 `lpath` 的 type 取出對應 layer 的 virtual transmitting node and virtual receiving node
- 再取出的 virtual transmitting node and virtual receiving node 之間建立新的 candidate link `c_link`
- 在 `c_link` 上記錄其所對應的 light path `lpath`
- 在 `lpath` 上記錄其所對應的 candidate link `c_link`
- 將 `lpath` 加入 candidate light path list

```c++
void build_light_path(Phy_graph& p_graph, LightPath* candidate_path, Aux_node* aux_source, Aux_node* aux_destination, Event& event);
```
實際在網路上將 candidate path 建立, 成為 exist light path
- 新建一個 light path object
- 在 `request2lightpath` 中記錄當前 request 和新建立的 light path 的關係
- assign transceiver 資源給新建的 light path
- 將新建 light path 使用的 slots mark as used and guard band
- 將新建的 light path 加入對應的 exist light path list

```c++
void path_parsing(Phy_graph& p_graph, Aux_node2Aux_link& result, Aux_node* aux_source, Aux_node* aux_destination, Event& event)
```
對輔助圖上找到的最佳路徑進行分析, 建立對應的 path 或是更新資源運用情況
- 一一處理所有最佳路徑所經過的 auxiliary links
- 如果經過 candidate link 代表需要新建 light path, call `build_light_path` 並建立 candidate link 所記錄的 light path
- 如果經過 spectrum link 代表 request 經過 exist OTDM light path, 如果經過的 light path 上沒有 request 的紀錄, 就把 request 加入 light path 並更新 `available_bitrate` (因為一個 request 可能經過同一個 OTDM light path 的不同個 spectrum link)
- 如果經過 grooming link 代表 request 經過了一次 OEO
- 如果經過 virtual adding link 代表 request 在還沒有 assign transmitter 的 OTDM light path intermediate node 進行 adding, 需要 assign 新的 transmitter
- 如果經過 virtual dropping link 代表 request 在還沒有 assign receiver 的 OTDM light path intermediate node 進行 dropping, 需要 assign 新的 receiver

```c++
int num_spectrum_available(Phy_link& link, int slot_st, int slot_ed)
```
計算一個 physical link 在指定的 slot 範圍內有多少的 available(free) slots 

```c++
int spectrum_available(Phy_link& link, int slot_st, int slot_ed)
```
測試指定的 link 的指定 slot 範圍是否都是 available(free) slots, 如果是回傳 `-1`, 否則回傳離 slot 0 距離最遠的 available slot, 這樣在找可用的 spectrum 時能讓上層的 function 知道下一個嘗試範圍要從哪開始

```c++
int path_spectrum_available(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph)
```
測試指定 physical path 的所有 link 的指定 slot 範圍是否都是 available(free) slots, 如果是回傳 `-1`, 否則回傳發現 used slot 的 link 上離 slot 0 距離最遠的 available slot, 這樣在找可用的 spectrum 時能讓上層的 function 知道下一個嘗試範圍要從哪開始

```c++
int get_distance(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph)
```
指定 physical path 上所有 link 的給定 slot 範圍周遭嘗試尋找 used slot (也就是其他的 light path), 並計算與其他 light path 之間的距離

```c++
int get_cut_num(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph)
```
計算指定 physical path 上的 cut 數量, 也就是給定 slot 範圍的兩旁是否是 available slot, 如果是 available slot, cut + 1

```c++
int get_align_num(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph)
```
對給定 physical path 上每一條 link 計算 alignment, 也就是給定 path 上 link 的 neighbor link 在給定 slot 範圍的 available slot 數

```c++
double weigh_path_spectrum(Path& path, int slot_st, int slot_ed, Phy_graph& p_graph)
```
對給定的 physical path 以及給訂的 slot 範圍計算出一個 weight 值

```c++
Spectrum find_best_spectrum(Path& path, int require_slots, Phy_graph& p_graph)
```
給定 physical path 以及需要的 slot 數, 利用 `path_spectrum_available()` 以及 `weigh_path_spectrum()` 找出 weight 最小的 spectrum, 如果沒有可用的 spectrum, 則回傳一個 `slot_st` 以及 `slot_ed` 為 `-1` 的 spectrum

```c++
LightPath* get_best_OTDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
```
對給定的 node pair 找出最好的 OTDM candidate light path
- 先確定在 source node 及 destination node 上有足夠的 OTDM transceiver 可以用來建立此條 light path
- 利用 `find_best_spectrum()` 找出最好的 spectrum, 比較不同 path 的 spectrum, 找出最好的 path spectrum 組合
- 建立新的 light path object 並回傳

```c++
int get_available_OFDM_transceiver(vector<OFDMTransceiver>& transceivers)
```
給定一個 transceiver vector, 回傳 id 最小的 available OFDM transceiver

```c++
LightPath* get_best_OFDM_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
```
對給定的 node pair 找出最好的 OFDM candidate light path
- 先確定在 source node 及 destination node 上有足夠的 OFDM transceiver 可以用來建立此條 light path
- 利用 `find_best_spectrum()` 找出最好的 spectrum, 並比較不同 path 的 spectrum, 找出最好的 path spectrum 組合
- 建立新的 light path object 並回傳
- 如果找不到可用的 light path 回傳 NULL

```c++
LightPath* get_best_OFDM_WB_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
```
對給定的 node pair 找出最好的 OFDM WB candidate light path
- 先確定在 destination node 上有足夠的 OFDM receiver 可以用來建立此條 light path
- 嘗試讓每一條 candidate physical path 與 exist OFDM light path 做 optical grooming (With Branch)
- 確定 exist OFDM light path 的 transmitter 有足夠的可用 slot 以及 sub-transceiver
- 依序比對 candidate physical path 與 exist OFDM light path 的 physical nodes 找出他們在哪個 physical node 分叉
- 將 candidate physical path 分成兩段(分叉前 -> trunk, 分叉後 -> branch)
- 往 exist OFDM light path 的周遭尋找可用的 spectrum (trunk 與 branch 的範圍不同), 並利用 `weigh_path_spectrum()` 找出最好的 spectrum (以 branch 的 slot 範圍計算)
- 建立新的 light path object 並回傳
- 如果找不到可用的 light path 回傳 NULL

```c++
LightPath* get_best_OFDM_WOB_light_path(int source, int destination, Event& event, Phy_graph& p_graph)
```
對給定的 node pair 找出最好的 OFDM WOB candidate light path
- 對每一條 exist OFDM light path 作嘗試
- 確定 exist OFDM light path 的 convener node 與指定的 source node 相同, end node 與指定的 destination node 相同
- 確定 exist OFDM light path 的 transceiver 有足夠的可用 slot 以及 sub-transceiver
- 往 exist OFDM light path 的周遭尋找可用的 spectrum, 並利用 `weigh_path_spectrum()` 找出最好的 spectrum
- 建立新的 light path object 並回傳
- 如果找不到可用的 light path 回傳 NULL

```c++
Aux_node2Aux_link BellmanFordSP(Aux_node* s)
```
Queue based BellmanFord 參考下列網頁
- http://algs4.cs.princeton.edu/44sp/
- http://algs4.cs.princeton.edu/44sp/BellmanFordSP.java.html

```c++
double get_dist(Aux_node2Double& distTo, Aux_node* node)
```
回傳從 source node 到給定 node 的最小 cost, 沒有 access 過的 node assign 並回傳 double 的最大值

```c++
void relax(Aux_node* v, Aux_node2Double& distTo, Aux_node2Aux_link& edgeTo, Aux_node2Bool& onQueue, queue<Aux_node*>& queue)
```
Queue based BellmanFord 參考下列網頁
- http://algs4.cs.princeton.edu/44sp/
- http://algs4.cs.princeton.edu/44sp/BellmanFordSP.java.html

```c++
void print_result(Traffic traffic)
```
產生檔案名稱, 並印出需要的資訊













