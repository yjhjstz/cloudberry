#include "transform.h"

const char* transform_result[TRANSFORM_NUM] = 
{

// 124
"####\n\
with customer_address_tmp as\n\
(select ca_country, ca_zip, ca_state, count(*) as sum_cus\n\
from customer_address \n\
group by ca_country, ca_zip, ca_state\n\
)\n\
,ssales as\n\
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
      ,sum(#### * sum_cus) netpaid\n\
from store_sales\n\
    ,store_returns\n\
    ,store\n\
    ,item\n\
    ,customer\n\
    ,customer_address_tmp\n\
where ss_ticket_number = sr_ticket_number\n\
  and ss_item_sk = sr_item_sk\n\
  and ss_customer_sk = c_customer_sk\n\
  and ss_item_sk = i_item_sk\n\
  and ss_store_sk = s_store_sk\n\
  and c_birth_country = upper(ca_country)\n\
  and s_zip = ca_zip\n\
and s_market_id=####\n\
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
where i_color = '####'\n\
group by c_last_name\n\
        ,c_first_name\n\
        ,s_store_name\n\
having sum(netpaid) > (select 0.05*avg(netpaid)\n\
                                 from ssales)\n\
;",

// 123-1
"####\n\
with frequent_ss_items as \n\
 (select substr(i_item_desc,1,30) itemdesc,i_item_sk item_sk,\n\
 d_date solddate,count(*) cnt\n\
  from store_sales\n\
      ,date_dim \n\
      ,item\n\
  where ss_sold_date_sk = d_date_sk\n\
    and ss_item_sk = i_item_sk \n\
    and d_year in (####,####+1,####+2,####+3)\n\
  group by substr(i_item_desc,1,30),i_item_sk,d_date\n\
  having count(*) >4),\n\
 max_store_sales as\n\
 (select max(csales) tpcds_cmax \n\
  from (select c_customer_sk,sum(ss_agg_tmp) csales\n\
        from customer, (\n\
            select ss_customer_sk ss_key_tmp, \n\
            sum(ss_quantity*ss_sales_price) ss_agg_tmp \n\
            from store_sales, date_dim \n\
            where ss_sold_date_sk = d_date_sk\n\
            and d_year in (####,####+1,####+2,####+3)\n\
            group by ss_customer_sk\n\
        ) ss_table_tmp\n\
        where c_customer_sk = ss_key_tmp\n\
        group by c_customer_sk) x)\n\
,\n\
 best_ss_customer as\n\
 (select c_customer_sk,sum(ss_agg_tmp) ssales\n\
  from customer, (\n\
    select ss_customer_sk ss_key_tmp, \n\
    sum(ss_quantity*ss_sales_price) ss_agg_tmp \n\
    from store_sales group by ss_customer_sk\n\
  ) ss_table_tmp\n\
  where c_customer_sk = ss_key_tmp\n\
  group by c_customer_sk\n\
  having sum(ss_agg_tmp) > (####/100.0) * (select\n\
  *\n\
from\n\
 max_store_sales))\n\
  select  sum(sales)\n\
 from (select cs_quantity*cs_list_price sales\n\
       from catalog_sales\n\
           ,date_dim \n\
       where d_year = #### \n\
         and d_moy = #### \n\
         and cs_sold_date_sk = d_date_sk \n\
         and cs_item_sk in (select item_sk from frequent_ss_items)\n\
         and cs_bill_customer_sk in \n\
         (select c_customer_sk from best_ss_customer)\n\
      union all\n\
      select ws_quantity*ws_list_price sales\n\
       from web_sales \n\
           ,date_dim \n\
       where d_year = #### \n\
         and d_moy = #### \n\
         and ws_sold_date_sk = d_date_sk \n\
         and ws_item_sk in (select item_sk from frequent_ss_items)\n\
         and ws_bill_customer_sk in \n\
         (select c_customer_sk from best_ss_customer)) x\n\
 ####;",

// 123-2
"####\n\
with frequent_ss_items as\n\
 (select substr(i_item_desc,1,30) itemdesc,i_item_sk item_sk,d_date solddate,count(*) cnt\n\
  from store_sales\n\
      ,date_dim\n\
      ,item\n\
  where ss_sold_date_sk = d_date_sk\n\
    and ss_item_sk = i_item_sk\n\
    and d_year in (####,#### + 1,#### + 2,#### + 3)\n\
  group by substr(i_item_desc,1,30),i_item_sk,d_date\n\
  having count(*) >4),\n\
 max_store_sales as\n\
 (select max(csales) tpcds_cmax\n\
  from (select c_customer_sk,sum(ss_agg_tmp) csales\n\
        from customer, (\n\
            select ss_customer_sk ss_key_tmp, sum(ss_quantity*ss_sales_price) ss_agg_tmp from store_sales, date_dim \n\
            where ss_sold_date_sk = d_date_sk\n\
            and d_year in (####,####+1,####+2,####+3)\n\
            group by ss_customer_sk\n\
        ) ss_table_tmp\n\
        where c_customer_sk = ss_key_tmp\n\
        group by c_customer_sk) x),\n\
 best_ss_customer as\n\
 (select c_customer_sk,sum(ss_agg_tmp) ssales\n\
  from customer, (\n\
    select ss_customer_sk ss_key_tmp, sum(ss_quantity*ss_sales_price) ss_agg_tmp from store_sales group by ss_customer_sk\n\
  ) ss_table_tmp\n\
  where c_customer_sk = ss_key_tmp\n\
  group by c_customer_sk\n\
  having sum(ss_agg_tmp) > (####/100.0) * (select\n\
  *\n\
 from max_store_sales))\n\
  select  c_last_name,c_first_name,sales\n\
 from (select c_last_name,c_first_name,sum(cs_quantity*cs_list_price) sales\n\
        from catalog_sales\n\
            ,customer\n\
            ,date_dim \n\
        where d_year = #### \n\
         and d_moy = #### \n\
         and cs_sold_date_sk = d_date_sk \n\
         and cs_item_sk in (select item_sk from frequent_ss_items)\n\
         and cs_bill_customer_sk in (select c_customer_sk from best_ss_customer)\n\
         and cs_bill_customer_sk = c_customer_sk \n\
       group by c_last_name,c_first_name\n\
      union all\n\
      select c_last_name,c_first_name,sum(ws_quantity*ws_list_price) sales\n\
       from web_sales\n\
           ,customer\n\
           ,date_dim \n\
       where d_year = #### \n\
         and d_moy = #### \n\
         and ws_sold_date_sk = d_date_sk \n\
         and ws_item_sk in (select item_sk from frequent_ss_items)\n\
         and ws_bill_customer_sk in (select c_customer_sk from best_ss_customer)\n\
         and ws_bill_customer_sk = c_customer_sk\n\
       group by c_last_name,c_first_name) x\n\
     order by c_last_name,c_first_name,sales\n\
  ####;",

// 195
"####with ws_wh as\n\
(select distinct(ws1.ws_order_number)\n\
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
    d_date between '####-####-01' and \n\
           (cast('####-####-01' as date) + '60 days'::interval)\n\
and ws1.ws_ship_date_sk = d_date_sk\n\
and ws1.ws_ship_addr_sk = ca_address_sk\n\
and ca_state = '####'\n\
and ws1.ws_web_site_sk = web_site_sk\n\
and web_company_name = 'pri'\n\
and ws1.ws_order_number in (select ws_order_number\n\
                            from ws_wh)\n\
and ws1.ws_order_number in (select wr_order_number\n\
                            from web_returns,ws_wh\n\
                            where wr_order_number = ws_wh.ws_order_number)\n\
order by count(distinct ws_order_number)\n\
####;"
,

// 104
"####with year_total as (\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,c_preferred_cust_flag customer_preferred_cust_flag\n\
       ,c_birth_country customer_birth_country\n\
       ,c_login customer_login\n\
       ,c_email_address customer_email_address\n\
       ,d_year dyear\n\
       ,sum(year_tmp_total) year_total\n\
       ,'s' sale_type\n\
 from customer\n\
     ,(select sum(((ss_ext_list_price-ss_ext_wholesale_cost-ss_ext_discount_amt)+ss_ext_sales_price)/2) year_tmp_total,\n\
     ss_customer_sk,\n\
     d_year\n\
     from store_sales,date_dim\n\
     where ss_sold_date_sk = d_date_sk\n\
     group by ss_customer_sk, d_year\n\
     ) store_sales_tmp\n\
 where c_customer_sk = ss_customer_sk    \n\
 group by c_customer_id\n\
         ,c_first_name\n\
         ,c_last_name\n\
         ,c_preferred_cust_flag\n\
         ,c_birth_country\n\
         ,c_login\n\
         ,c_email_address\n\
         ,dyear\n\
 union all\n\
select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,c_preferred_cust_flag customer_preferred_cust_flag\n\
       ,c_birth_country customer_birth_country\n\
       ,c_login customer_login\n\
       ,c_email_address customer_email_address\n\
       ,d_year dyear\n\
       ,sum(year_tmp_total) year_total\n\
       ,'c' sale_type\n\
 from customer\n\
     ,(select sum((((cs_ext_list_price-cs_ext_wholesale_cost-cs_ext_discount_amt)+cs_ext_sales_price)/2) ) year_tmp_total,\n\
     cs_bill_customer_sk,\n\
     d_year\n\
     from catalog_sales,date_dim\n\
     where cs_sold_date_sk = d_date_sk\n\
     group by cs_bill_customer_sk, d_year\n\
     ) catalog_sales_tmp\n\
 where c_customer_sk = cs_bill_customer_sk\n\
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
       ,sum(year_tmp_total) year_total\n\
       ,'w' sale_type\n\
 from customer\n\
     ,(select sum((((ws_ext_list_price-ws_ext_wholesale_cost-ws_ext_discount_amt)+ws_ext_sales_price)/2) ) year_tmp_total,\n\
     ws_bill_customer_sk,\n\
     d_year\n\
     from web_sales,date_dim\n\
     where ws_sold_date_sk = d_date_sk\n\
     group by ws_bill_customer_sk, d_year\n\
     ) web_sales_tmp\n\
 where c_customer_sk = ws_bill_customer_sk\n\
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
                 ,####\n\
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
   and t_s_firstyear.dyear =  ####\n\
   and t_s_secyear.dyear = ####+1\n\
   and t_c_firstyear.dyear =  ####\n\
   and t_c_secyear.dyear =  ####+1\n\
   and t_w_firstyear.dyear = ####\n\
   and t_w_secyear.dyear = ####+1\n\
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
         ,####\n\
####;"
,

// 111
"####with year_total as (\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,c_preferred_cust_flag customer_preferred_cust_flag\n\
       ,c_birth_country customer_birth_country\n\
       ,c_login customer_login\n\
       ,c_email_address customer_email_address\n\
       ,d_year dyear\n\
       ,sum(year_tmp_total) year_total\n\
       ,'s' sale_type\n\
 from customer\n\
    ,(select sum(ss_ext_list_price-ss_ext_discount_amt) year_tmp_total,\n\
     ss_customer_sk,\n\
     d_year\n\
     from store_sales,date_dim\n\
     where ss_sold_date_sk = d_date_sk\n\
     group by ss_customer_sk, d_year\n\
     ) store_sales_tmp\n\
 where c_customer_sk = ss_customer_sk\n\
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
       ,sum(year_tmp_total) year_total\n\
       ,'w' sale_type\n\
 from customer\n\
     ,(select sum(ws_ext_list_price-ws_ext_discount_amt) year_tmp_total,\n\
     ws_bill_customer_sk,\n\
     d_year\n\
     from web_sales,date_dim\n\
     where ws_sold_date_sk = d_date_sk\n\
     group by ws_bill_customer_sk, d_year\n\
     ) web_sales_tmp\n\
 where c_customer_sk = ws_bill_customer_sk\n\
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
                 ,####\n\
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
         and t_s_firstyear.dyear = ####\n\
         and t_s_secyear.dyear = ####+1\n\
         and t_w_firstyear.dyear = ####\n\
         and t_w_secyear.dyear = ####+1\n\
         and t_s_firstyear.year_total > 0\n\
         and t_w_firstyear.year_total > 0\n\
         and case when t_w_firstyear.year_total > 0 then t_w_secyear.year_total / t_w_firstyear.year_total else 0.0 end\n\
             > case when t_s_firstyear.year_total > 0 then t_s_secyear.year_total / t_s_firstyear.year_total else 0.0 end\n\
 order by t_s_secyear.customer_id\n\
         ,t_s_secyear.customer_first_name\n\
         ,t_s_secyear.customer_last_name\n\
         ,####\n\
####;"
,

// 178
"####with ws as\n\
  (select d_year AS ws_sold_year, ws_item_sk,\n\
    ws_bill_customer_sk ws_customer_sk,\n\
    sum(ws_quantity) ws_qty,\n\
    sum(ws_wholesale_cost) ws_wc,\n\
    sum(ws_sales_price) ws_sp\n\
   from web_sales\n\
   left join web_returns on wr_order_number=ws_order_number and ws_item_sk=wr_item_sk\n\
   join date_dim on ws_sold_date_sk = d_date_sk\n\
   where wr_order_number is null and d_year=2001\n\
   group by d_year, ws_item_sk, ws_bill_customer_sk\n\
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
   join ws on (ws_sold_year=d_year and ws_item_sk=ss_item_sk and ws_customer_sk=ss_customer_sk)\n\
   where sr_ticket_number is null\n\
   group by d_year, ss_item_sk, ss_customer_sk\n\
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
   join ss on (ss_sold_year=d_year and cs_item_sk=cs_item_sk and ss_customer_sk=cs_bill_customer_sk)\n\
   where cr_order_number is null\n\
   group by d_year, cs_item_sk, cs_bill_customer_sk\n\
   )\n\
 select \n\
####,\n\
round(ss_qty/(coalesce(ws_qty+cs_qty,1)),2) ratio,\n\
ss_qty store_qty, ss_wc store_wholesale_cost, ss_sp store_sales_price,\n\
coalesce(ws_qty,0)+coalesce(cs_qty,0) other_chan_qty,\n\
coalesce(ws_wc,0)+coalesce(cs_wc,0) other_chan_wholesale_cost,\n\
coalesce(ws_sp,0)+coalesce(cs_sp,0) other_chan_sales_price\n\
from ss\n\
join ws on (ws_sold_year=ss_sold_year and ws_item_sk=ss_item_sk and ws_customer_sk=ss_customer_sk)\n\
join cs on (cs_sold_year=ss_sold_year and cs_item_sk=cs_item_sk and cs_customer_sk=ss_customer_sk)\n\
where coalesce(ws_qty,0)>0 and coalesce(cs_qty, 0)>0 and ss_sold_year=####\n\
order by \n\
  ####,\n\
  ss_qty desc, ss_wc desc, ss_sp desc,\n\
  other_chan_qty,\n\
  other_chan_wholesale_cost,\n\
  other_chan_sales_price,\n\
  round(ss_qty/(coalesce(ws_qty+cs_qty,1)),2)\n\
####;"
,

// 174
"####with year_total as (\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,d_year as year\n\
       ,####(year_tmp_total) year_total\n\
       ,'s' sale_type\n\
 from customer\n\
     ,(select ####(ss_net_paid) year_tmp_total,\n\
     ss_customer_sk,\n\
     d_year\n\
     from store_sales,date_dim\n\
     where ss_sold_date_sk = d_date_sk\n\
     group by ss_customer_sk, d_year\n\
     ) store_sales_tmp\n\
 where c_customer_sk = ss_customer_sk\n\
   and d_year in (####,####+1)\n\
 group by c_customer_id\n\
         ,c_first_name\n\
         ,c_last_name\n\
         ,d_year\n\
 union all\n\
 select c_customer_id customer_id\n\
       ,c_first_name customer_first_name\n\
       ,c_last_name customer_last_name\n\
       ,d_year as year\n\
       ,####(year_tmp_total) year_total\n\
       ,'w' sale_type\n\
 from customer\n\
     ,(select ####(ws_net_paid) year_tmp_total,\n\
     ws_bill_customer_sk,\n\
     d_year\n\
     from web_sales,date_dim\n\
     where ws_sold_date_sk = d_date_sk\n\
     group by ws_bill_customer_sk, d_year\n\
     ) web_sales_tmp\n\
 where c_customer_sk = ws_bill_customer_sk\n\
   and d_year in (####,####+1)\n\
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
         and t_s_firstyear.year = ####\n\
         and t_s_secyear.year = ####+1\n\
         and t_w_firstyear.year = ####\n\
         and t_w_secyear.year = ####+1\n\
         and t_s_firstyear.year_total > 0\n\
         and t_w_firstyear.year_total > 0\n\
         and case when t_w_firstyear.year_total > 0 then t_w_secyear.year_total / t_w_firstyear.year_total else null end\n\
           > case when t_s_firstyear.year_total > 0 then t_s_secyear.year_total / t_s_firstyear.year_total else null end\n\
 order by ####,####,####\n\
####;"
};

