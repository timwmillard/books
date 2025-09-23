
insert into account 
    (number, type, name, normal_balance) 
values 
    ('1000', 'asset', 'Cash','debit'),
    ('1100', 'asset', 'Accounts Receivable', 'debit'),
    ('2000', 'liability', 'Accounts Payable', 'credit'),
    ('3000', 'equity', 'Owner''s Equity', 'credit'),
    ('4000', 'revenue', 'Revenue', 'credit'),
    ('4100', 'revenue', 'Sales', 'credit'),
    ('5000', 'expense', 'Expenses', 'debit');

-- create table if not exists account (
--     id bigserial primary key,
--     number text not null,
--     account_type text check (account_type in ('asset', 'liability', 'equity', 'revenue', 'expense')) not null,
--     name text not null,
--     description text not null default '',
--     normal_balance text check (normal_balance in ('debit', 'credit')) not null,
--     status text check (status in ('active', 'closed')) not null default 'active',
--     business_id bigint references business(id)
-- );
