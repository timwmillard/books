
create table if not exists business (
    id integer primary key check (id = 1),
    name text not null,
    business_type text,
    owners_name text,
    start_date timestamptz,
    end_date timestamptz,
    abn text,
    date_format text not null default 'DD/MM/YY',
    currency text not null default 'AUD',
    gst_registered bool not null default false,
    goods_sold bool not null default false
);

create table if not exists account (
    id integer primary key,
    type text check (type in ('asset', 'liability', 'equity', 'revenue', 'expense')) not null,
    name text not null,
    description text not null default '',
    normal_balance text check (normal_balance in ('debit', 'credit')) not null,
    status text check (status in ('active', 'closed')) not null default 'active',
    parent_id integer references account(id)
);

create table if not exists open_balance (
    account_id integer not null references account(id),
    balance integer not null default 0,
    from_date timestamptz
);

create table if not exists journal (
    id integer primary key,
    date date not null,
    description text,
    reference text,  -- invoice #, check #, etc.
    created_at timestamp not null default current_timestamp
);

create table if not exists journal_line (
    id integer primary key,
    journal_id integer not null references journal(id),
    account_id integer not null references account(id),
    amount integer not null
);

drop view if exists ledger;
create view ledger as
select
    line.id as line_id,
    entry.id as journal_id,
    entry.date,
    line.account_id,
    line.amount,
    entry.description,
    entry.reference,
    entry.created_at
from journal entry
join journal_line line on line.journal_id = entry.id
order by entry.date, entry.id, line.id;

drop view if exists general_ledger;
create view general_ledger as
select
    line.id as line_id,
    entry.id as journal_id,
    entry.date,
    line.account_id,
    account.name as account_name,
    account.type as account_type,
    case when (amount > 0) then line.amount else 0 end as debit,
    case when (amount < 0) then -line.amount else 0 end as credit,
    entry.description,
    entry.reference,
    entry.created_at
from journal entry
join journal_line line on line.journal_id = entry.id
join account on line.account_id = account.id
order by entry.date, entry.id, line.id;

-- Bank Reconciliation
create table if not exists external_account (
    id integer primary key,
    name text not null,
    description text not null default '',
    status text check (status in ('active', 'closed')) not null default 'active',
    account_id integer not null references account(id)
);

create table if not exists external_transaction (
    id integer primary key,
    date date not null,
    amount integer not null,
    description text not null default '',
    external_account_id integer not null references external_account(id),
    journal_entry_id integer references journal_entry(id),
    unique_id text, -- TODO: should include external_account_id
    unique(external_account_id, unique_id)
);

