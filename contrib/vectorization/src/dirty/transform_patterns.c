#include "transform.h"

const char* transform_pattern[TRANSFORM_NUM] = 
{
  
// 124
"\\(.*\\)with ssales as\n\
(select c_last_name\n\
      ,c_first_name\n\
      ,s_store_name\n\
      ,ca_state\n\
      ,s_state\n\
      ,i_color\n\
      ,i_current_price\n\
      ,i_manager_id\n\
      ,i_units\n\
      ,i_size\n\
      ,sum(\\(.*\\)) netpaid\n\
from store_sales\n\
    ,store_returns\n\
    ,store\n\
    ,item\n\
    ,customer\n\
    ,customer_address\n\
where ss_ticket_number = sr_ticket_number\n\
  and ss_item_sk = sr_item_sk\n\
  and ss_customer_sk = c_customer_sk\n\
  and ss_item_sk = i_item_sk\n\
  and ss_store_sk = s_store_sk\n\
  and c_birth_country = upper(ca_country)\n\
  and s_zip = ca_zip\n\
\\s*and s_market_id\\s*=\\s*\\(.*\\)\n\
group by c_last_name\n\
        ,c_first_name\n\
        ,s_store_name\n\
        ,ca_state\n\
        ,s_state\n\
        ,i_color\n\
        ,i_current_price\n\
        ,i_manager_id\n\
        ,i_units\n\
        ,i_size)\n\
select c_last_name\n\
      ,c_first_name\n\
      ,s_store_name\n\
      ,sum(netpaid) paid\n\
from ssales\n\
where i_color = '\\(.*\\)'\n\
group by c_last_name\n\
        ,c_first_name\n\
        ,s_store_name\n\
having sum(netpaid) > (select 0.05\\*avg(netpaid)\n\
\\s*from ssales)\n\
;"
,

// 123-1
"\\(.*\\)with frequent_ss_items as \n\
 (select substr(i_item_desc,1,30) itemdesc,i_item_sk item_sk,d_date solddate,count(\\*) cnt\n\
  from store_sales\n\
      ,date_dim \n\
      ,item\n\
  where ss_sold_date_sk = d_date_sk\n\
    and ss_item_sk = i_item_sk \n\
    and d_year in (\\(.*\\),.*+1,.*+2,.*+3)\n\
  group by substr(i_item_desc,1,30),i_item_sk,d_date\n\
  having count(\\*) >4),\n\
 max_store_sales as\n\
 (select max(csales) tpcds_cmax \n\
  from (select c_customer_sk,sum(ss_quantity\\*ss_sales_price) csales\n\
        from store_sales\n\
            ,customer\n\
            ,date_dim \n\
        where ss_customer_sk = c_customer_sk\n\
         and ss_sold_date_sk = d_date_sk\n\
         and d_year in (.*,.*+1,.*+2,.*+3) \n\
        group by c_customer_sk) x),\n\
 best_ss_customer as\n\
 (select c_customer_sk,sum(ss_quantity\\*ss_sales_price) ssales\n\
  from store_sales\n\
      ,customer\n\
  where ss_customer_sk = c_customer_sk\n\
  group by c_customer_sk\n\
  having sum(ss_quantity\\*ss_sales_price) > (\\(.*\\)/100.0) \\* (select\n\
  \\*\n\
from\n\
 max_store_sales))\n\
  select  sum(sales)\n\
 from (select cs_quantity\\*cs_list_price sales\n\
       from catalog_sales\n\
           ,date_dim \n\
       where d_year = .* \n\
         and d_moy = \\(.*\\) \n\
         and cs_sold_date_sk = d_date_sk \n\
         and cs_item_sk in (select item_sk from frequent_ss_items)\n\
         and cs_bill_customer_sk in (select c_customer_sk from best_ss_customer)\n\
      union all\n\
      select ws_quantity\\*ws_list_price sales\n\
       from web_sales \n\
           ,date_dim \n\
       where d_year = .* \n\
         and d_moy = .* \n\
         and ws_sold_date_sk = d_date_sk \n\
         and ws_item_sk in (select item_sk from frequent_ss_items)\n\
         and ws_bill_customer_sk in (select c_customer_sk from best_ss_customer)) x\n\
 \\(.*\\);"
,

// 123-2
"\\(.*\\)with frequent_ss_items as\n\
 (select substr(i_item_desc,1,30) itemdesc,i_item_sk item_sk,d_date solddate,count(\\*) cnt\n\
  from store_sales\n\
      ,date_dim\n\
      ,item\n\
  where ss_sold_date_sk = d_date_sk\n\
    and ss_item_sk = i_item_sk\n\
    and d_year in (\\(.*\\),.* + 1,.* + 2,.* + 3)\n\
  group by substr(i_item_desc,1,30),i_item_sk,d_date\n\
  having count(\\*) >4),\n\
 max_store_sales as\n\
 (select max(csales) tpcds_cmax\n\
  from (select c_customer_sk,sum(ss_quantity\\*ss_sales_price) csales\n\
        from store_sales\n\
            ,customer\n\
            ,date_dim \n\
        where ss_customer_sk = c_customer_sk\n\
         and ss_sold_date_sk = d_date_sk\n\
         and d_year in (.*,.*+1,.*+2,.*+3)\n\
        group by c_customer_sk) x),\n\
 best_ss_customer as\n\
 (select c_customer_sk,sum(ss_quantity\\*ss_sales_price) ssales\n\
  from store_sales\n\
      ,customer\n\
  where ss_customer_sk = c_customer_sk\n\
  group by c_customer_sk\n\
  having sum(ss_quantity\\*ss_sales_price) > (\\(.*\\)/100.0) \\* (select\n\
  \\*\n\
 from max_store_sales))\n\
  select  c_last_name,c_first_name,sales\n\
 from (select c_last_name,c_first_name,sum(cs_quantity\\*cs_list_price) sales\n\
        from catalog_sales\n\
            ,customer\n\
            ,date_dim \n\
        where d_year = .* \n\
         and d_moy = \\(.*\\) \n\
         and cs_sold_date_sk = d_date_sk \n\
         and cs_item_sk in (select item_sk from frequent_ss_items)\n\
         and cs_bill_customer_sk in (select c_customer_sk from best_ss_customer)\n\
         and cs_bill_customer_sk = c_customer_sk \n\
       group by c_last_name,c_first_name\n\
      union all\n\
      select c_last_name,c_first_name,sum(ws_quantity\\*ws_list_price) sales\n\
       from web_sales\n\
           ,customer\n\
           ,date_dim \n\
       where d_year = .* \n\
         and d_moy = .* \n\
         and ws_sold_date_sk = d_date_sk \n\
         and ws_item_sk in (select item_sk from frequent_ss_items)\n\
         and ws_bill_customer_sk in (select c_customer_sk from best_ss_customer)\n\
         and ws_bill_customer_sk = c_customer_sk\n\
       group by c_last_name,c_first_name) x\n\
     order by c_last_name,c_first_name,sales\n\
  \\(.*\\);",

// 195
"\\(.*\\)with ws_wh as\n\
(select ws1.ws_order_number,ws1.ws_warehouse_sk wh1,ws2.ws_warehouse_sk wh2\n\
 from web_sales ws1,web_sales ws2\n\
 where ws1.ws_order_number = ws2.ws_order_number\n\
   and ws1.ws_warehouse_sk <> ws2.ws_warehouse_sk)\n\
 select  \n\
   count(distinct ws_order_number) as \"order count\"\n\
  ,sum(ws_ext_ship_cost) as \"total shipping cost\"\n\
  ,sum(ws_net_profit) as \"total net profit\"\n\
from\n\
   web_sales ws1\n\
  ,date_dim\n\
  ,customer_address\n\
  ,web_site\n\
where\n\
    d_date between '\\(.*\\)-\\(.*\\)-01' and \n\
           (cast('.*-.*-01' as date) + '60 days'::interval)\n\
and ws1.ws_ship_date_sk = d_date_sk\n\
and ws1.ws_ship_addr_sk = ca_address_sk\n\
and ca_state = '\\(.*\\)'\n\
and ws1.ws_web_site_sk = web_site_sk\n\
and web_company_name = 'pri'\n\
and ws1.ws_order_number in (select ws_order_number\n\
                            from ws_wh)\n\
and ws1.ws_order_number in (select wr_order_number\n\
                            from web_returns,ws_wh\n\
                            where wr_order_number = ws_wh.ws_order_number)\n\
order by count(distinct ws_order_number)\n\
\\(.*\\);"
,

// 104
"\\(.*\\)with year_total as (\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,c_preferred_cust_flag customer_preferred_cust_flag\n\
       ,c_birth_country customer_birth_country\n\
       ,c_login customer_login\n\
       ,c_email_address customer_email_address\n\
       ,d_year dyear\n\
       ,sum(((ss_ext_list_price-ss_ext_wholesale_cost-ss_ext_discount_amt)+ss_ext_sales_price)/2) year_total\n\
       ,'s' sale_type\n\
 from customer\n\
     ,store_sales\n\
     ,date_dim\n\
 where c_customer_sk = ss_customer_sk\n\
   and ss_sold_date_sk = d_date_sk\n\
 group by c_customer_id\n\
         ,c_first_name\n\
         ,c_last_name\n\
         ,c_preferred_cust_flag\n\
         ,c_birth_country\n\
         ,c_login\n\
         ,c_email_address\n\
         ,d_year\n\
 union all\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,c_preferred_cust_flag customer_preferred_cust_flag\n\
       ,c_birth_country customer_birth_country\n\
       ,c_login customer_login\n\
       ,c_email_address customer_email_address\n\
       ,d_year dyear\n\
       ,sum((((cs_ext_list_price-cs_ext_wholesale_cost-cs_ext_discount_amt)+cs_ext_sales_price)/2) ) year_total\n\
       ,'c' sale_type\n\
 from customer\n\
     ,catalog_sales\n\
     ,date_dim\n\
 where c_customer_sk = cs_bill_customer_sk\n\
   and cs_sold_date_sk = d_date_sk\n\
 group by c_customer_id\n\
         ,c_first_name\n\
         ,c_last_name\n\
         ,c_preferred_cust_flag\n\
         ,c_birth_country\n\
         ,c_login\n\
         ,c_email_address\n\
         ,d_year\n\
union all\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,c_preferred_cust_flag customer_preferred_cust_flag\n\
       ,c_birth_country customer_birth_country\n\
       ,c_login customer_login\n\
       ,c_email_address customer_email_address\n\
       ,d_year dyear\n\
       ,sum((((ws_ext_list_price-ws_ext_wholesale_cost-ws_ext_discount_amt)+ws_ext_sales_price)/2) ) year_total\n\
       ,'w' sale_type\n\
 from customer\n\
     ,web_sales\n\
     ,date_dim\n\
 where c_customer_sk = ws_bill_customer_sk\n\
   and ws_sold_date_sk = d_date_sk\n\
 group by c_customer_id\n\
         ,c_first_name\n\
         ,c_last_name\n\
         ,c_preferred_cust_flag\n\
         ,c_birth_country\n\
         ,c_login\n\
         ,c_email_address\n\
         ,d_year\n\
         )\n\
  select  \n\
                  t_s_secyear.customer_id\n\
                 ,t_s_secyear.customer_first_name\n\
                 ,t_s_secyear.customer_last_name\n\
                 ,\\(.*\\)\n\
 from year_total t_s_firstyear\n\
     ,year_total t_s_secyear\n\
     ,year_total t_c_firstyear\n\
     ,year_total t_c_secyear\n\
     ,year_total t_w_firstyear\n\
     ,year_total t_w_secyear\n\
 where t_s_secyear.customer_id = t_s_firstyear.customer_id\n\
   and t_s_firstyear.customer_id = t_c_secyear.customer_id\n\
   and t_s_firstyear.customer_id = t_c_firstyear.customer_id\n\
   and t_s_firstyear.customer_id = t_w_firstyear.customer_id\n\
   and t_s_firstyear.customer_id = t_w_secyear.customer_id\n\
   and t_s_firstyear.sale_type = 's'\n\
   and t_c_firstyear.sale_type = 'c'\n\
   and t_w_firstyear.sale_type = 'w'\n\
   and t_s_secyear.sale_type = 's'\n\
   and t_c_secyear.sale_type = 'c'\n\
   and t_w_secyear.sale_type = 'w'\n\
   and t_s_firstyear.dyear =  \\(.*\\)\n\
   and t_s_secyear.dyear = .*+1\n\
   and t_c_firstyear.dyear =  .*\n\
   and t_c_secyear.dyear =  .*+1\n\
   and t_w_firstyear.dyear = .*\n\
   and t_w_secyear.dyear = .*+1\n\
   and t_s_firstyear.year_total > 0\n\
   and t_c_firstyear.year_total > 0\n\
   and t_w_firstyear.year_total > 0\n\
   and case when t_c_firstyear.year_total > 0 then t_c_secyear.year_total / t_c_firstyear.year_total else null end\n\
           > case when t_s_firstyear.year_total > 0 then t_s_secyear.year_total / t_s_firstyear.year_total else null end\n\
   and case when t_c_firstyear.year_total > 0 then t_c_secyear.year_total / t_c_firstyear.year_total else null end\n\
           > case when t_w_firstyear.year_total > 0 then t_w_secyear.year_total / t_w_firstyear.year_total else null end\n\
 order by t_s_secyear.customer_id\n\
         ,t_s_secyear.customer_first_name\n\
         ,t_s_secyear.customer_last_name\n\
         ,.*\n\
\\(.*\\);"
,

// 111
"\\(.*\\)with year_total as (\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,c_preferred_cust_flag customer_preferred_cust_flag\n\
       ,c_birth_country customer_birth_country\n\
       ,c_login customer_login\n\
       ,c_email_address customer_email_address\n\
       ,d_year dyear\n\
       ,sum(ss_ext_list_price-ss_ext_discount_amt) year_total\n\
       ,'s' sale_type\n\
 from customer\n\
     ,store_sales\n\
     ,date_dim\n\
 where c_customer_sk = ss_customer_sk\n\
   and ss_sold_date_sk = d_date_sk\n\
 group by c_customer_id\n\
         ,c_first_name\n\
         ,c_last_name\n\
         ,c_preferred_cust_flag \n\
         ,c_birth_country\n\
         ,c_login\n\
         ,c_email_address\n\
         ,d_year \n\
 union all\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,c_preferred_cust_flag customer_preferred_cust_flag\n\
       ,c_birth_country customer_birth_country\n\
       ,c_login customer_login\n\
       ,c_email_address customer_email_address\n\
       ,d_year dyear\n\
       ,sum(ws_ext_list_price-ws_ext_discount_amt) year_total\n\
       ,'w' sale_type\n\
 from customer\n\
     ,web_sales\n\
     ,date_dim\n\
 where c_customer_sk = ws_bill_customer_sk\n\
   and ws_sold_date_sk = d_date_sk\n\
 group by c_customer_id\n\
         ,c_first_name\n\
         ,c_last_name\n\
         ,c_preferred_cust_flag \n\
         ,c_birth_country\n\
         ,c_login\n\
         ,c_email_address\n\
         ,d_year\n\
         )\n\
  select  \n\
                  t_s_secyear.customer_id\n\
                 ,t_s_secyear.customer_first_name\n\
                 ,t_s_secyear.customer_last_name\n\
                 ,\\(.*\\)\n\
 from year_total t_s_firstyear\n\
     ,year_total t_s_secyear\n\
     ,year_total t_w_firstyear\n\
     ,year_total t_w_secyear\n\
 where t_s_secyear.customer_id = t_s_firstyear.customer_id\n\
         and t_s_firstyear.customer_id = t_w_secyear.customer_id\n\
         and t_s_firstyear.customer_id = t_w_firstyear.customer_id\n\
         and t_s_firstyear.sale_type = 's'\n\
         and t_w_firstyear.sale_type = 'w'\n\
         and t_s_secyear.sale_type = 's'\n\
         and t_w_secyear.sale_type = 'w'\n\
         and t_s_firstyear.dyear = \\(.*\\)\n\
         and t_s_secyear.dyear = .*+1\n\
         and t_w_firstyear.dyear = .*\n\
         and t_w_secyear.dyear = .*+1\n\
         and t_s_firstyear.year_total > 0\n\
         and t_w_firstyear.year_total > 0\n\
         and case when t_w_firstyear.year_total > 0 then t_w_secyear.year_total / t_w_firstyear.year_total else 0.0 end\n\
             > case when t_s_firstyear.year_total > 0 then t_s_secyear.year_total / t_s_firstyear.year_total else 0.0 end\n\
 order by t_s_secyear.customer_id\n\
         ,t_s_secyear.customer_first_name\n\
         ,t_s_secyear.customer_last_name\n\
         ,.*\n\
\\(.*\\);"
,

// 178
"\\(.*\\)with ws as\n\
  (select d_year AS ws_sold_year, ws_item_sk,\n\
    ws_bill_customer_sk ws_customer_sk,\n\
    sum(ws_quantity) ws_qty,\n\
    sum(ws_wholesale_cost) ws_wc,\n\
    sum(ws_sales_price) ws_sp\n\
   from web_sales\n\
   left join web_returns on wr_order_number=ws_order_number and ws_item_sk=wr_item_sk\n\
   join date_dim on ws_sold_date_sk = d_date_sk\n\
   where wr_order_number is null\n\
   group by d_year, ws_item_sk, ws_bill_customer_sk\n\
   ),\n\
cs as\n\
  (select d_year AS cs_sold_year, cs_item_sk,\n\
    cs_bill_customer_sk cs_customer_sk,\n\
    sum(cs_quantity) cs_qty,\n\
    sum(cs_wholesale_cost) cs_wc,\n\
    sum(cs_sales_price) cs_sp\n\
   from catalog_sales\n\
   left join catalog_returns on cr_order_number=cs_order_number and cs_item_sk=cr_item_sk\n\
   join date_dim on cs_sold_date_sk = d_date_sk\n\
   where cr_order_number is null\n\
   group by d_year, cs_item_sk, cs_bill_customer_sk\n\
   ),\n\
ss as\n\
  (select d_year AS ss_sold_year, ss_item_sk,\n\
    ss_customer_sk,\n\
    sum(ss_quantity) ss_qty,\n\
    sum(ss_wholesale_cost) ss_wc,\n\
    sum(ss_sales_price) ss_sp\n\
   from store_sales\n\
   left join store_returns on sr_ticket_number=ss_ticket_number and ss_item_sk=sr_item_sk\n\
   join date_dim on ss_sold_date_sk = d_date_sk\n\
   where sr_ticket_number is null\n\
   group by d_year, ss_item_sk, ss_customer_sk\n\
   )\n\
 select \n\
\\(.*\\),\n\
round(ss_qty/(coalesce(ws_qty+cs_qty,1)),2) ratio,\n\
ss_qty store_qty, ss_wc store_wholesale_cost, ss_sp store_sales_price,\n\
coalesce(ws_qty,0)+coalesce(cs_qty,0) other_chan_qty,\n\
coalesce(ws_wc,0)+coalesce(cs_wc,0) other_chan_wholesale_cost,\n\
coalesce(ws_sp,0)+coalesce(cs_sp,0) other_chan_sales_price\n\
from ss\n\
left join ws on (ws_sold_year=ss_sold_year and ws_item_sk=ss_item_sk and ws_customer_sk=ss_customer_sk)\n\
left join cs on (cs_sold_year=ss_sold_year and cs_item_sk=cs_item_sk and cs_customer_sk=ss_customer_sk)\n\
where coalesce(ws_qty,0)>0 and coalesce(cs_qty, 0)>0 and ss_sold_year=\\(.*\\)\n\
order by \n\
  .*,\n\
  ss_qty desc, ss_wc desc, ss_sp desc,\n\
  other_chan_qty,\n\
  other_chan_wholesale_cost,\n\
  other_chan_sales_price,\n\
  round(ss_qty/(coalesce(ws_qty+cs_qty,1)),2)\n\
\\(.*\\);"
,

// 174
"\\(.*\\)with year_total as (\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,d_year as year\n\
       ,\\(.*\\)(ss_net_paid) year_total\n\
       ,'s' sale_type\n\
 from customer\n\
     ,store_sales\n\
     ,date_dim\n\
 where c_customer_sk = ss_customer_sk\n\
   and ss_sold_date_sk = d_date_sk\n\
   and d_year in (\\(.*\\),.*+1)\n\
 group by c_customer_id\n\
         ,c_first_name\n\
         ,c_last_name\n\
         ,d_year\n\
 union all\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,d_year as year\n\
       ,.*(ws_net_paid) year_total\n\
       ,'w' sale_type\n\
 from customer\n\
     ,web_sales\n\
     ,date_dim\n\
 where c_customer_sk = ws_bill_customer_sk\n\
   and ws_sold_date_sk = d_date_sk\n\
   and d_year in (.*,.*+1)\n\
 group by c_customer_id\n\
         ,c_first_name\n\
         ,c_last_name\n\
         ,d_year\n\
         )\n\
  select \n\
        t_s_secyear.customer_id, t_s_secyear.customer_first_name, t_s_secyear.customer_last_name\n\
 from year_total t_s_firstyear\n\
     ,year_total t_s_secyear\n\
     ,year_total t_w_firstyear\n\
     ,year_total t_w_secyear\n\
 where t_s_secyear.customer_id = t_s_firstyear.customer_id\n\
         and t_s_firstyear.customer_id = t_w_secyear.customer_id\n\
         and t_s_firstyear.customer_id = t_w_firstyear.customer_id\n\
         and t_s_firstyear.sale_type = 's'\n\
         and t_w_firstyear.sale_type = 'w'\n\
         and t_s_secyear.sale_type = 's'\n\
         and t_w_secyear.sale_type = 'w'\n\
         and t_s_firstyear.year = .*\n\
         and t_s_secyear.year = .*+1\n\
         and t_w_firstyear.year = .*\n\
         and t_w_secyear.year = .*+1\n\
         and t_s_firstyear.year_total > 0\n\
         and t_w_firstyear.year_total > 0\n\
         and case when t_w_firstyear.year_total > 0 then t_w_secyear.year_total / t_w_firstyear.year_total else null end\n\
           > case when t_s_firstyear.year_total > 0 then t_s_secyear.year_total / t_s_firstyear.year_total else null end\n\
 order by \\(.*\\),\\(.*\\),\\(.*\\)\n\
\\(.*\\);"
};

const char* merge_join_partten[3] =
{
// q151
"\\(.*\\)WITH web_v1 as (\n\
select\n\
  ws_item_sk item_sk, d_date,\n\
  sum(sum(ws_sales_price))\n\
      over (partition by ws_item_sk order by d_date rows between unbounded preceding and current row) cume_sales\n\
from web_sales\n\
    ,date_dim\n\
where ws_sold_date_sk=d_date_sk\n\
  and d_month_seq between \\(.*\\) and \\(.*\\)+11\n\
  and ws_item_sk is not NULL\n\
group by ws_item_sk, d_date),\n\
store_v1 as (\n\
select\n\
  ss_item_sk item_sk, d_date,\n\
  sum(sum(ss_sales_price))\n\
      over (partition by ss_item_sk order by d_date rows between unbounded preceding and current row) cume_sales\n\
from store_sales\n\
    ,date_dim\n\
where ss_sold_date_sk=d_date_sk\n\
  and d_month_seq between \\(.*\\) and \\(.*\\)+11\n\
  and ss_item_sk is not NULL\n\
group by ss_item_sk, d_date)\n\
\\(.*\\) select \\(.*\\) \\*\n\
from (select item_sk\n\
     ,d_date\n\
     ,web_sales\n\
     ,store_sales\n\
     ,max(web_sales)\n\
         over (partition by item_sk order by d_date rows between unbounded preceding and current row) web_cumulative\n\
     ,max(store_sales)\n\
         over (partition by item_sk order by d_date rows between unbounded preceding and current row) store_cumulative\n\
     from (select case when web.item_sk is not null then web.item_sk else store.item_sk end item_sk\n\
                 ,case when web.d_date is not null then web.d_date else store.d_date end d_date\n\
                 ,web.cume_sales web_sales\n\
                 ,store.cume_sales store_sales\n\
           from web_v1 web full outer join store_v1 store on (web.item_sk = store.item_sk\n\
                                                          and web.d_date = store.d_date)\n\
          )x )y\n\
where web_cumulative > store_cumulative\n\
order by item_sk\n\
        ,d_date\n\
\\(.*\\);"
,
// q197
"\\(.*\\)with ssci as (\n\
select ss_customer_sk customer_sk\n\
      ,ss_item_sk item_sk\n\
from store_sales,date_dim\n\
where ss_sold_date_sk = d_date_sk\n\
  and d_month_seq between \\(.*\\) and \\(.*\\) + 11\n\
group by ss_customer_sk\n\
        ,ss_item_sk),\n\
csci as(\n\
 select cs_bill_customer_sk customer_sk\n\
      ,cs_item_sk item_sk\n\
from catalog_sales,date_dim\n\
where cs_sold_date_sk = d_date_sk\n\
  and d_month_seq between \\(.*\\) and \\(.*\\) + 11\n\
group by cs_bill_customer_sk\n\
        ,cs_item_sk)\n\
\\(.*\\) select \\(.*\\) sum(case when ssci.customer_sk is not null and csci.customer_sk is null then 1 else 0 end) store_only\n\
      ,sum(case when ssci.customer_sk is null and csci.customer_sk is not null then 1 else 0 end) catalog_only\n\
      ,sum(case when ssci.customer_sk is not null and csci.customer_sk is not null then 1 else 0 end) store_and_catalog\n\
from ssci full outer join csci on (ssci.customer_sk=csci.customer_sk\n\
                               and ssci.item_sk = csci.item_sk)\n\
\\(.*\\);"
,
NULL,
};

const char* partition_top_k_partten =
//167
".*select \\(.*\\) \\*\n\
from (select i_category\n\
            ,i_class\n\
            ,i_brand\n\
            ,i_product_name\n\
            ,d_year\n\
            ,d_qoy\n\
            ,d_moy\n\
            ,s_store_id\n\
            ,sumsales\n\
            ,rank() over (partition by i_category order by sumsales desc) rk\n\
      from (select i_category\n\
                  ,i_class\n\
                  ,i_brand\n\
                  ,i_product_name\n\
                  ,d_year\n\
                  ,d_qoy\n\
                  ,d_moy\n\
                  ,s_store_id\n\
                  ,sum(coalesce(ss_sales_price\\*ss_quantity,0)) sumsales\n\
            from store_sales\n\
                ,date_dim\n\
                ,store\n\
                ,item\n\
       where  ss_sold_date_sk=d_date_sk\n\
          and ss_item_sk=i_item_sk\n\
          and ss_store_sk = s_store_sk\n\
          and d_month_seq between \\(.*\\) and \\(.*\\)+11\n\
       group by  rollup(i_category, i_class, i_brand, i_product_name, d_year, d_qoy, d_moy,s_store_id))dw1) dw2\n\
where rk <= 100\n\
order by i_category\n\
        ,i_class\n\
        ,i_brand\n\
        ,i_product_name\n\
        ,d_year\n\
        ,d_qoy\n\
        ,d_moy\n\
        ,s_store_id\n\
        ,sumsales\n\
        ,rk\n\
\\(.*\\);"
;

const char* stddev_partten[3] = 
{
// 139
"\\(.*\\)with inv as\n\
(select w_warehouse_name,w_warehouse_sk,i_item_sk,d_moy\n\
       ,stdev,mean, case mean when 0 then null else stdev/mean end cov\n\
 from(select w_warehouse_name,w_warehouse_sk,i_item_sk,d_moy\n\
            ,stddev_samp(inv_quantity_on_hand) stdev,avg(inv_quantity_on_hand) mean\n\
      from inventory\n\
          ,item\n\
          ,warehouse\n\
          ,date_dim\n\
      where inv_item_sk = i_item_sk\n\
        and inv_warehouse_sk = w_warehouse_sk\n\
        and inv_date_sk = d_date_sk\n\
        and d_year =2001\n\
      group by w_warehouse_name,w_warehouse_sk,i_item_sk,d_moy) foo\n\
 where case mean when 0 then 0 else stdev/mean end > 1)\n\
select inv1.w_warehouse_sk,inv1.i_item_sk,inv1.d_moy,inv1.mean, inv1.cov\n\
        ,inv2.w_warehouse_sk,inv2.i_item_sk,inv2.d_moy,inv2.mean, inv2.cov\n\
from inv inv1,inv inv2\n\
where inv1.i_item_sk = inv2.i_item_sk\n\
  and inv1.w_warehouse_sk =  inv2.w_warehouse_sk\n\
  and inv1.d_moy=1\n\
  and inv2.d_moy=1+1\n\
order by inv1.w_warehouse_sk,inv1.i_item_sk,inv1.d_moy,inv1.mean,inv1.cov\n\
        ,inv2.d_moy,inv2.mean, inv2.cov\n\
\\(.*\\);",
"\\(.*\\)with inv as\n\
(select w_warehouse_name,w_warehouse_sk,i_item_sk,d_moy\n\
       ,stdev,mean, case mean when 0 then null else stdev/mean end cov\n\
 from(select w_warehouse_name,w_warehouse_sk,i_item_sk,d_moy\n\
            ,stddev_samp(inv_quantity_on_hand) stdev,avg(inv_quantity_on_hand) mean\n\
      from inventory\n\
          ,item\n\
          ,warehouse\n\
          ,date_dim\n\
      where inv_item_sk = i_item_sk\n\
        and inv_warehouse_sk = w_warehouse_sk\n\
        and inv_date_sk = d_date_sk\n\
        and d_year =2001\n\
      group by w_warehouse_name,w_warehouse_sk,i_item_sk,d_moy) foo\n\
 where case mean when 0 then 0 else stdev/mean end > 1)\n\
select inv1.w_warehouse_sk,inv1.i_item_sk,inv1.d_moy,inv1.mean, inv1.cov\n\
        ,inv2.w_warehouse_sk,inv2.i_item_sk,inv2.d_moy,inv2.mean, inv2.cov\n\
from inv inv1,inv inv2\n\
where inv1.i_item_sk = inv2.i_item_sk\n\
  and inv1.w_warehouse_sk =  inv2.w_warehouse_sk\n\
  and inv1.d_moy=1\n\
  and inv2.d_moy=1+1\n\
  and inv1.cov > 1.5\n\
order by inv1.w_warehouse_sk,inv1.i_item_sk,inv1.d_moy,inv1.mean,inv1.cov\n\
        ,inv2.d_moy,inv2.mean, inv2.cov\n\
\\(.*\\);",
NULL,
};

const char* seventy_two_control_partten =
// 172
".*select  i_item_desc\n\
      ,w_warehouse_name\n\
      ,d1.d_week_seq\n\
      ,sum(case when p_promo_sk is null then 1 else 0 end) no_promo\n\
      ,sum(case when p_promo_sk is not null then 1 else 0 end) promo\n\
      ,count(\\*) total_cnt\n\
from catalog_sales\n\
join inventory on (cs_item_sk = inv_item_sk)\n\
join warehouse on (w_warehouse_sk=inv_warehouse_sk)\n\
join item on (i_item_sk = cs_item_sk)\n\
join customer_demographics on (cs_bill_cdemo_sk = cd_demo_sk)\n\
join household_demographics on (cs_bill_hdemo_sk = hd_demo_sk)\n\
join date_dim d1 on (cs_sold_date_sk = d1.d_date_sk)\n\
join date_dim d2 on (inv_date_sk = d2.d_date_sk)\n\
join date_dim d3 on (cs_ship_date_sk = d3.d_date_sk)\n\
left outer join promotion on (cs_promo_sk=p_promo_sk)\n\
left outer join catalog_returns on (cr_item_sk = cs_item_sk and cr_order_number = cs_order_number)\n\
where d1.d_week_seq = d2.d_week_seq\n\
  and inv_quantity_on_hand < cs_quantity \n\
  and d3.d_date > d1.d_date + .*\n\
  and hd_buy_potential = .*\n\
  and d1.d_year = .*\n\
  and cd_marital_status = .*\n\
group by i_item_desc,w_warehouse_name,d1.d_week_seq\n\
order by total_cnt desc, i_item_desc, w_warehouse_name, d_week_seq\n\
limit 100\\(.*\\);"
;